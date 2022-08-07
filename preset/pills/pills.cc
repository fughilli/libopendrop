#include "preset/pills/pills.h"

#include <algorithm>
#include <cmath>

#include "debug/control_injector.h"
#include "debug/signal_scope.h"
#include "preset/common/outline_model.h"
#include "preset/pills/composite.fsh.h"
#include "preset/pills/passthrough_frag.fsh.h"
#include "preset/pills/passthrough_vert.vsh.h"
#include "preset/pills/warp.fsh.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/enums.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"
#include "util/math/math.h"
#include "util/math/perspective.h"
#include "util/math/vector.h"
#include "util/signal/signals.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
std::tuple<int, float> CountAndScale(float arg, int max_count) {
  int n_clusters = 1.0f + std::fmod(arg, max_count);
  float cluster_scale = (cos(arg * 2.0f * kPi) + 1.0f) / 2.0f;
  return std::make_tuple(n_clusters, cluster_scale);
}
}  // namespace

Pills::Pills(std::shared_ptr<gl::GlProgram> warp_program,
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
      outline_model_(outline_model)

{}

absl::StatusOr<std::shared_ptr<Preset>> Pills::MakeShared(
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

  return std::shared_ptr<Pills>(
      new Pills(warp_program, composite_program, passthrough_program, nullptr,
                front_render_target, back_render_target, depth_output_target,
                outline_model, texture_manager));
}

void Pills::OnUpdateGeometry() {
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

void Pills::DrawCubes(float power, float bass, float energy, float dt,
                      float time, float zoom_coeff, glm::vec3 zoom_vec,
                      int num_cubes) {
  float cube_scale = SIGINJECT_OVERRIDE(
      "pills_model_scale",
      static_cast<float>(
          std::clamp(0.1 + (cos(energy * 20) + 1) / 15.5, 0.0, 2.0) * 0.3 +
          bass / 5),
      0.05f, 0.4f);

  float rot_speed_coeff =
      SIGINJECT_OVERRIDE("pills_rot_speed_coeff", 1.0f, 0.0f, 5.0f);

  rot_arg_ += rot_speed_coeff * dt;

  float SIGPLOT_ASSIGN(locate_mix_coeff,
                       SineEase(std::clamp(zoom_coeff * 3, 0.0f, 1.0f)));

  auto [n_clusters, cluster_scale] = CountAndScale(time / 3, 7);

  SIGPLOT_ON("cluster_scale", cluster_scale);
  SIGPLOT_ON("maybe_sign", Sign(cos(time / 3 * kPi)));

  glm::mat4 model_transform;
  for (int i = 0; i < num_cubes; ++i) {
    float cluster_coeff = 0.0f;
    float position_along_ring = static_cast<float>(
        sin(energy) * 5 + ClusterDistribution(i, num_cubes, n_clusters,
                                              cluster_scale, &cluster_coeff));
    float SIGPLOT_ASSIGN(
        ring_radius,
        SIGINJECT_OVERRIDE("pills_radius", 0.3f + (sin(energy * 10) + 1) / 6,
                           0.0f, 1.0f));
    ring_radius *= Lerp((0.5f + cluster_coeff * 0.5f), 1.0f, cluster_scale);
    glm::mat4 ring_transform = RingTransform(Directions::kOutOfScreen,
                                             ring_radius, position_along_ring);

    // Build a transform that takes an object from the center and moves it out
    // along a line extending from the left to the right edge of the display.
    float distribute_coeff = 2 * i / static_cast<float>(num_cubes);
    position_accum_ +=
        (dt / 30) * SineEase(-std::clamp(zoom_coeff * 3, -1.0f, 0.0f));
    float SIGPLOT_ASSIGN_WRAP(slide_coeff, position_accum_, 0.0f, 1.0f);
    glm::mat4 line_transform = glm::translate(
        glm::mat4(1.0f),
        // Rotate the set of objects from one side of the screen to the other.
        // Choose the extents such that in the worst case we don't have a gap at
        // either end.
        glm::vec3(
            Rotate2d(
                glm::vec2(
                    (-1.0f + std::fmodf(distribute_coeff + slide_coeff, 2.0f)) *
                        (static_cast<float>(num_cubes + 1) / (num_cubes - 1)),
                    0.0f),
                kPi * 2 * ring_radius),
            0.0f));
    glm::mat4 locate_transform =
        TransformMix(line_transform, ring_transform, locate_mix_coeff);

    model_transform = locate_transform *
                      glm::mat4x4(cube_scale * 0.7, 0, 0, 0,  // Row 1
                                  0, cube_scale * 0.7, 0, 0,  // Row 2
                                  0, 0, cube_scale * 0.7, 0,  // Row 3
                                  0, 0, 0, 1                  // Row 4
                                  ) *
                      glm::rotate(glm::mat4(1.0f), rot_arg_ * 1.0f,
                                  glm::vec3(0.0f, 0.0f, 1.0f)) *
                      glm::rotate(glm::mat4(1.0f), rot_arg_ * 0.7f,
                                  glm::vec3(0.0f, 1.0f, 0.0f)) *
                      glm::rotate(glm::mat4(1.0f), rot_arg_ * 1.5f,
                                  glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::vec4 color_a = glm::vec4(HsvToRgb(glm::vec3(energy, 1, 1)), 1);
    const glm::vec4 color_b =
        glm::vec4(HsvToRgb(glm::vec3(energy + 0.5, 1, 1)), 1);

    if (SIGINJECT_TRIGGER("pills_texture_enable")) {
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

void Pills::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float bass = SIGPLOT("bass power", state->bass());
  SIGPLOT("mid power", state->mid());
  SIGPLOT("treble power", state->treble());

  float SIGPLOT_ASSIGN_WRAP(energy, static_cast<float>(state->energy()), 0.0f,
                            1.0f);
  energy *= 0.1;
  float SIGPLOT_ASSIGN(power, state->power());

  float SIGPLOT_ASSIGN(
      zoom_coeff,
      SIGINJECT_OVERRIDE("pills_zoom_coeff", sin(energy * 2), -1.0f, 1.0f));

  float SIGPLOT_ASSIGN(trunc_zoom_coeff,
                       std::clamp(SineEase(abs(zoom_coeff)), 0.0f, 1.0f));

  int num_cubes = SIGINJECT_OVERRIDE("pills_num_cubes", 16, 0, 32);

  glm::vec3 zoom_vec =
      glm::vec3(UnitVectorAtAngle(energy * 2) *
                    SIGINJECT_OVERRIDE("pills_zoom_dir_scale",
                                       std::sin(energy * 0.3f), -1.0f, 1.0f),
                0) *
          trunc_zoom_coeff +
      Directions::kIntoScreen;
  glm::vec3 cube_orient_vec = zoom_vec;
  cube_orient_vec.x /= 2;
  cube_orient_vec.y /= 2;

  {
    auto depth_output_activation = depth_output_target_->Activate();
    glViewport(0, 0, longer_dimension(), longer_dimension());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthRange(0, 10);
    glEnable(GL_DEPTH_TEST);
    DrawCubes(power, bass, energy, state->dt(), state->t(), zoom_coeff,
              cube_orient_vec, num_cubes);
    glDisable(GL_DEPTH_TEST);
  }

  {
    auto front_activation = front_render_target_->Activate();

    auto program_activation = warp_program_->Activate();

    zoom_coeff =
        (SineEase(MapValue<float, /*clamp=*/true>(
             (zoom_coeff - 1.0f / 3.0f) * 2.0f, -1.0f, 1.0f, 0.0f, 1.0f)) *
             2.0f -
         1.0f) *
            0.1 +
        1.05;
    SIGPLOT("shader zoom coeff", zoom_coeff);

    GlBindUniform(warp_program_, "frame_size", glm::ivec2(width(), height()));
    GlBindUniform(warp_program_, "power", power);
    GlBindUniform(warp_program_, "energy", energy);
    // Figure out how to keep it from zooming towards the viewer when the line
    // is moving
    GlBindUniform(warp_program_, "zoom_coeff", zoom_coeff);
    GlBindUniform(warp_program_, "zoom_vec", zoom_vec);
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));
    auto binding_options = gl::GlTextureBindingOptions();
    background_hue_ +=
        power * SIGINJECT_OVERRIDE("pills_border_hue_coeff", 0.1f, 0.0f, 0.5f);
    binding_options.border_color = glm::vec4(
        HsvToRgb(glm::vec3(
            background_hue_, 1,
            SIGINJECT_OVERRIDE("pills_border_value_coeff", 1.0f, 0.0f, 1.0f))),
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
