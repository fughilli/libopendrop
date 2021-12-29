#include "preset/cube_wreath/cube_wreath.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "preset/cube_wreath/composite.fsh.h"
#include "preset/cube_wreath/cube.obj.h"
#include "preset/cube_wreath/cube_outline.obj.h"
#include "preset/cube_wreath/model.fsh.h"
#include "preset/cube_wreath/passthrough_frag.fsh.h"
#include "preset/cube_wreath/passthrough_vert.vsh.h"
#include "preset/cube_wreath/warp.fsh.h"
#include "util/colors.h"
#include "util/gl_util.h"
#include "util/logging.h"
#include "util/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 2.0f;
}

CubeWreath::CubeWreath(std::shared_ptr<gl::GlProgram> warp_program,
                       std::shared_ptr<gl::GlProgram> composite_program,
                       std::shared_ptr<gl::GlProgram> model_program,
                       std::shared_ptr<gl::GlProgram> passthrough_program,
                       std::shared_ptr<gl::GlRenderTarget> model_texture_target,
                       std::shared_ptr<gl::GlRenderTarget> front_render_target,
                       std::shared_ptr<gl::GlRenderTarget> back_render_target,
                       std::shared_ptr<gl::GlRenderTarget> depth_output_target,
                       std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      model_program_(model_program),
      passthrough_program_(passthrough_program),
      model_texture_target_(model_texture_target),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target),
      depth_output_target_(depth_output_target),
      cube_(cube_obj::Vertices(), cube_obj::Normals(), cube_obj::Uvs(),
            cube_obj::Triangles()),
      cube_outline_(cube_outline_obj::Vertices(), cube_outline_obj::Normals(),
                    cube_outline_obj::Uvs(), cube_outline_obj::Triangles()) {}

absl::StatusOr<std::shared_ptr<Preset>> CubeWreath::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(auto warp_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto model_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             model_fsh::Code()));
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

  return std::shared_ptr<CubeWreath>(
      new CubeWreath(warp_program, composite_program, model_program,
                     passthrough_program, nullptr, front_render_target,
                     back_render_target, depth_output_target, texture_manager));
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

void CubeWreath::DrawCubes(float power, float energy) {
  constexpr static int kNumCubes = 16;

  float cube_scale = std::clamp(0.1 + (cos(energy * 20) + 1) / 15.5, 0.0, 2.0);

  glm::mat4 model_transform;
  for (int i = 0; i < kNumCubes; ++i) {
    model_transform =
        glm::rotate(
            glm::mat4(1.0f),
            static_cast<float>(sin(energy) * 5 + (i * M_PI * 2.0f / kNumCubes)),
            glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.5f + (sin(energy * 10) + 1) / 5, 0.0f, 0.0f)) *
        glm::mat4x4(cube_scale, 0, 0, 0,  // Row 1
                    0, cube_scale, 0, 0,  // Row 2
                    0, 0, cube_scale, 0,  // Row 3
                    0, 0, 0, 1            // Row 4
                    ) *
        glm::rotate(glm::mat4(1.0f), energy * 10, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), energy * 7, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), energy * 15, glm::vec3(1.0f, 0.0f, 0.0f));
    GlBindUniform(model_program_, "energy", energy);
    GlBindUniform(model_program_, "model_transform", model_transform);
    GlBindUniform(model_program_, "black", false);
    GlBindUniform(model_program_, "light_color_a",
                  glm::vec4(HsvToRgb(glm::vec3(energy, 1, 1)), 1));
    GlBindUniform(model_program_, "light_color_b",
                  glm::vec4(HsvToRgb(glm::vec3(energy + 0.5, 1, 1)), 1));
    cube_.Draw();

    GlBindUniform(model_program_, "black", true);
    cube_outline_.Draw();
  }
}

void CubeWreath::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy();
  float power = state->power();

  auto buffer_size = samples.size() / 2;
  vertices_.resize(buffer_size);

  float cos_energy = cos(energy * 10 + power * 10);
  float sin_energy = sin(energy * 10 + power * 10);
  float total_scale_factor = kScaleFactor * (0.7 + 0.3 * cos_energy);

  for (int i = 0; i < buffer_size; ++i) {
    float x_scaled = samples[i * 2] * total_scale_factor;
    float y_scaled = samples[i * 2 + 1] * total_scale_factor;

    float x_pos = x_scaled * cos_energy - y_scaled * sin_energy;
    float y_pos = x_scaled * sin_energy + y_scaled * cos_energy;

    vertices_[i] = glm::vec2(x_pos, y_pos);
  }

  {
    auto depth_output_activation = depth_output_target_->Activate();
    auto program_activation = model_program_->Activate();

    GlBindUniform(model_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(model_program_, "alpha", alpha);
    GlBindRenderTargetTextureToUniform(model_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());

    glViewport(0, 0, longer_dimension(), longer_dimension());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthRange(0, 10);
    glEnable(GL_DEPTH_TEST);
    DrawCubes(power, energy);
    glDisable(GL_DEPTH_TEST);
  }

  {
    auto front_activation = front_render_target_->Activate();

    auto program_activation = warp_program_->Activate();

    GlBindUniform(warp_program_, "frame_size", glm::ivec2(width(), height()));
    GlBindUniform(warp_program_, "power", power);
    GlBindUniform(warp_program_, "energy", energy);
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));
    auto binding_options = gl::GlTextureBindingOptions();
    binding_options.border_color = glm::vec4(1, 1, 0, 1);
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
