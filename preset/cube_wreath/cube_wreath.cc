#include "preset/cube_wreath/cube_wreath.h"

#include <algorithm>
#include <cmath>

#include "debug/control_injector.h"
#include "preset/common/outline_model.h"
#include "preset/cube_wreath/composite.fsh.h"
#include "preset/cube_wreath/passthrough_frag.fsh.h"
#include "preset/cube_wreath/passthrough_vert.vsh.h"
#include "preset/cube_wreath/warp.fsh.h"
#include "util/graphics/colors.h"
#include "util/enums.h"
#include "third_party/gl_helper.h"
#include "util/graphics/gl_util.h"
#include "third_party/glm_helper.h"
#include "util/logging/logging.h"
#include "util/math/perspective.h"
#include "util/math/vector.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 2.0f;
}

CubeWreath::CubeWreath(std::shared_ptr<gl::GlProgram> warp_program,
                       std::shared_ptr<gl::GlProgram> composite_program,
                       std::shared_ptr<gl::GlProgram> passthrough_program,
                       std::shared_ptr<gl::GlRenderTarget> model_texture_target,
                       std::shared_ptr<gl::GlRenderTarget> front_render_target,
                       std::shared_ptr<gl::GlRenderTarget> back_render_target,
                       std::shared_ptr<gl::GlRenderTarget> depth_output_target,
                       std::shared_ptr<OutlineModel> outline_model,
                       std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      passthrough_program_(passthrough_program),
      model_texture_target_(model_texture_target),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target),
      depth_output_target_(depth_output_target),
      outline_model_(outline_model) {}

absl::StatusOr<std::shared_ptr<Preset>> CubeWreath::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(auto warp_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto passthrough_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             passthrough_frag_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto depth_output_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager,
                                                  {.enable_depth = true}));
  ASSIGN_OR_RETURN(auto outline_model, OutlineModel::MakeShared());

  return std::shared_ptr<CubeWreath>(
      new CubeWreath(warp_program, composite_program, passthrough_program,
                     nullptr, front_render_target, back_render_target,
                     depth_output_target, outline_model, texture_manager));
}

void CubeWreath::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (model_texture_target_ != nullptr) {
    model_texture_target_->UpdateGeometry(longer_dimension(),
                                          longer_dimension());
  }
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(longer_dimension(),
                                         longer_dimension());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(longer_dimension(), longer_dimension());
  }
  if (depth_output_target_ != nullptr) {
    depth_output_target_->UpdateGeometry(longer_dimension(),
                                         longer_dimension());
  }
}

void CubeWreath::DrawCubes(float power, float energy, glm::vec3 zoom_vec,
                           int num_cubes) {
  float cube_scale =
      SIGINJECT_OVERRIDE("cube_wreath_model_scale",
                         static_cast<float>(std::clamp(
                             0.1 + (cos(energy * 20) + 1) / 15.5, 0.0, 2.0)),
                         0.05f, 0.4f);

  float rot_speed_coeff =
      SIGINJECT_OVERRIDE("cube_wreath_rot_speed_coeff", 1.0f, 0.0f, 5.0f);

  rot_arg_ += rot_speed_coeff * power / 50;

  glm::mat3x3 look_rotation = OrientTowards(zoom_vec);

  glm::mat4 model_transform;
  for (int i = 0; i < num_cubes; ++i) {
    model_transform =
        glm::mat4(look_rotation) *
        glm::rotate(
            glm::mat4(1.0f),
            static_cast<float>(sin(energy) * 5 + (i * M_PI * 2.0f / num_cubes)),
            glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(SIGINJECT_OVERRIDE("cube_wreath_radius",
                                         0.5f + (sin(energy * 10) + 1) / 5,
                                         0.0f, 1.0f),
                      0.0f, 0.0f)) *
        glm::mat4x4(cube_scale, 0, 0, 0,  // Row 1
                    0, cube_scale, 0, 0,  // Row 2
                    0, 0, cube_scale, 0,  // Row 3
                    0, 0, 0, 1            // Row 4
                    ) *
        glm::rotate(glm::mat4(1.0f), rot_arg_ * 10,
                    glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), rot_arg_ * 7,
                    glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), rot_arg_ * 15,
                    glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec4 color_a = glm::vec4(HsvToRgb(glm::vec3(energy, 1, 1)), 1);
    glm::vec4 color_b = glm::vec4(HsvToRgb(glm::vec3(energy + 0.5, 1, 1)), 1);

    if (SIGINJECT_TRIGGER("cube_wreath_texture_enable")) {
      texture_trigger_ = !texture_trigger_;
    }

    outline_model_->Draw({
        .model_transform = model_transform,
        .color_a = color_a,
        .color_b = color_b,
        .render_target = back_render_target_,
        .alpha = 1,
        .energy = energy,
        .blend_coeff = texture_trigger_ ? 0.3f : 0.0f,
        .model_to_draw = InterpolateEnum<OutlineModel::ModelToDraw>(
            std::fmodf(energy, 1.0f)),
    });
  }
}

void CubeWreath::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy();
  float power = state->power();

  glm::vec3 zoom_vec =
      glm::vec3(UnitVectorAtAngle(energy * 2) *
                    SIGINJECT_OVERRIDE("cube_wreath_zoom_dir_scale",
                                       std::sin(energy * 0.3f), -1.0f, 1.0f),
                0) +
      Directions::kIntoScreen;
  glm::vec3 cube_orient_vec = zoom_vec;
  cube_orient_vec.x /= 2;
  cube_orient_vec.y /= 2;
  float zoom_speed = SIGINJECT_OVERRIDE("cube_wreath_zoom_speed",
                                        power / 10.0f + 1.2f, 0.9f, 1.5f);

  int num_cubes = SIGINJECT_OVERRIDE("cube_wreath_num_cubes", 16, 1, 32);

  {
    auto depth_output_activation = depth_output_target_->Activate();

    glViewport(0, 0, longer_dimension(), longer_dimension());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthRange(0, 10);
    glEnable(GL_DEPTH_TEST);
    DrawCubes(power, energy, cube_orient_vec, num_cubes);
    glDisable(GL_DEPTH_TEST);
  }

  {
    auto front_activation = front_render_target_->Activate();

    auto program_activation = warp_program_->Activate();

    GlBindUniform(warp_program_, "frame_size", glm::ivec2(width(), height()));
    GlBindUniform(warp_program_, "power", power);
    GlBindUniform(warp_program_, "energy", energy);
    GlBindUniform(warp_program_, "zoom_vec", zoom_vec);
    GlBindUniform(warp_program_, "zoom_speed", zoom_speed);
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));
    auto binding_options = gl::GlTextureBindingOptions();

    background_hue_ +=
        power *
        SIGINJECT_OVERRIDE("cube_wreath_border_hue_coeff", 0.1f, 0.0f, 3.0f);
    binding_options.border_color = glm::vec4(
        HsvToRgb(glm::vec3(background_hue_, 1,
                           SIGINJECT_OVERRIDE("cube_wreath_border_value_coeff",
                                              1.0f, 0.0f, 1.0f))),
        1);
    binding_options.sampling_mode = gl::GlTextureSamplingMode::kClampToBorder;
    GlBindRenderTargetTextureToUniform(warp_program_, "last_frame",
                                       back_render_target_, binding_options);
    GlBindRenderTargetTextureToUniform(warp_program_, "input",
                                       depth_output_target_, binding_options);

    glViewport(0, 0, longer_dimension(), longer_dimension());
    rectangle_.Draw();
  }

  {
    auto output_activation = output_render_target->Activate();
    auto program_activation = composite_program_->Activate();

    GlBindUniform(composite_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(composite_program_, "model_transform", glm::mat4(1.0f));
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());

    SquareViewport();
    rectangle_.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
  }
}

}  // namespace opendrop
