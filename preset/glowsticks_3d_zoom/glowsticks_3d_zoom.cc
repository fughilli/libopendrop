#include "preset/glowsticks_3d_zoom/glowsticks_3d_zoom.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "absl/types/span.h"
#include "debug/control_injector.h"
#include "debug/signal_scope.h"
#include "preset/glowsticks_3d_zoom/composite.fsh.h"
#include "preset/glowsticks_3d_zoom/model.fsh.h"
#include "preset/glowsticks_3d_zoom/model.vsh.h"
#include "preset/glowsticks_3d_zoom/passthrough.vsh.h"
#include "preset/glowsticks_3d_zoom/ribbon.fsh.h"
#include "preset/glowsticks_3d_zoom/warp.fsh.h"
#include "util/coefficients.h"
#include "util/colors.h"
#include "util/gl_helper.h"
#include "util/gl_util.h"
#include "util/glm_helper.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/status_macros.h"

namespace opendrop {

namespace {
// Whether or not to draw the segments of the rotating armatures that describe
// the motion of the ribbon.
constexpr bool kDrawDebugSegments = false;

// Number of segments to use for the ribbon. This, together with the speed of
// the ribbon, determines the maximum length on the screen.
constexpr int kRibbonSegmentCount = 100;

// The maximum angle change, in radians, that can occur per frame for the
// armature.
constexpr float kMaxAngularStep = 0.04f;

// When true, the ribbon width is scaled by the instantaneous audio power,
// resulting in a waveform of the audio power appearing along the length of the
// ribbon.
constexpr bool kEnableRibbonWidthPowerScaling = false;

// How close the ribbon segment has to be to the horizontal centerline of the
// screen for a flip about that line to occur.
constexpr float kFlipTolerance = 0.01f;

// How much time must elapse between flips.
constexpr float kFlipMinimumInterval = 0.2f;

// Returns a unit vector rotated counter-clockwise from the +X unit vector by an
// angle in radians.
glm::vec2 UnitVectorAtAngle(float angle) {
  return glm::vec2(cos(angle), sin(angle));
}
}  // namespace

Glowsticks3dZoom::Glowsticks3dZoom(
    std::shared_ptr<gl::GlProgram> warp_program,
    std::shared_ptr<gl::GlProgram> ribbon_program,
    std::shared_ptr<gl::GlProgram> composite_program,
    std::shared_ptr<gl::GlRenderTarget> front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_render_target,
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      ribbon_program_(ribbon_program),
      composite_program_(composite_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target),
      ribbon_(glm::vec3(), kRibbonSegmentCount),
      ribbon2_(glm::vec3(), kRibbonSegmentCount),
      flip_y_(false),
      flip_oneshot_(kFlipMinimumInterval) {
  segment_scales_ = Coefficients::Random<3>(0.1, 0.5);
  segment_scales_[2] = 1.0f;

  auto base_position_array = Coefficients::Random<2>(-0.2f, 0.2f);
  base_position_ = glm::vec2(base_position_array[0], base_position_array[1]);
  color_coefficients_ = Coefficients::Random<2>(1.0f, 3.0f);
  direction_reversal_coefficients_ = Coefficients::Random<kNumSegments>(5, 20);
  rotational_rate_coefficients_ =
      Coefficients::Random<kNumSegments>(0.1f, 0.3f);
}

absl::StatusOr<std::shared_ptr<Preset>> Glowsticks3dZoom::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(auto warp_program, gl::GlProgram::MakeShared(
                                          model_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto ribbon_program,
      gl::GlProgram::MakeShared(model_vsh::Code(), ribbon_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager,
                                                  {.enable_depth = true}));
  return std::shared_ptr<Glowsticks3dZoom>(new Glowsticks3dZoom(
      warp_program, ribbon_program, composite_program, front_render_target,
      back_render_target, texture_manager));
}

void Glowsticks3dZoom::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());

  const auto square_dimension = std::max(width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(square_dimension, square_dimension);
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(square_dimension, square_dimension);
  }
}

void Glowsticks3dZoom::UpdateArmatureSegmentAngles(
    std::shared_ptr<GlobalState> state,
    std::array<Accumulator<float>, kNumSegments>* segment_angles) {
  float average_power = state->average_power();
  float energy = state->energy();
  float power = state->power();

  for (int i = 0; i < segment_angles->size(); ++i) {
    (*segment_angles)[i] += rotational_rate_coefficients_[i] *
                            SafeDivide(power, average_power) *
                            sin(energy * direction_reversal_coefficients_[i]);
  }
}

std::pair<glm::vec2, glm::vec2> Glowsticks3dZoom::ComputeRibbonSegment(
    std::shared_ptr<GlobalState> state,
    const std::array<float, kNumSegments> segment_angles,
    std::array<glm::vec2, kNumSegments + 1>* debug_segment_points) {
  float energy = state->energy();

  constexpr float kRibbonSegmentOffset = 0.12f;
  std::array<glm::vec2, kNumSegments> segments;

  for (int i = 0; i < segment_angles.size(); ++i) {
    segments[i] = UnitVectorAtAngle(segment_angles[i]) * segment_scales_[i];
  }

  float ribbon_width = 0.1 + 0.034 * sin(energy * 10);
  if (kEnableRibbonWidthPowerScaling) {
    ribbon_width += state->power() / 10;
  }

  *debug_segment_points = {base_position_, base_position_ + segments[0],
                           base_position_ + segments[0] + segments[1],
                           base_position_ + segments[0] + segments[1] +
                               segments[2] * kRibbonSegmentOffset};

  return {base_position_ + segments[0] + segments[1] +
              segments[2] * kRibbonSegmentOffset,
          base_position_ + segments[0] + segments[1] +
              segments[2] * (kRibbonSegmentOffset + ribbon_width)};
}

void Glowsticks3dZoom::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float average_power = state->average_power();
  float energy = state->energy();
  float normalized_energy = state->normalized_energy();
  float power = state->power();

  std::array<glm::vec2, 4> debug_segment_points;

  UpdateArmatureSegmentAngles(state, &segment_angle_accumulators_);
  // Determine how many steps to divide the arc into, by finding the minumum
  // number of subdivisions that achieves a maximum angular step of
  // `kMaxAngluarStep` for the fastest-changing angle.
  std::array<float, 3> angles_;
  for (int i = 0; i < angles_.size(); ++i) {
    angles_[i] = segment_angle_accumulators_[i];
  }
  int num_steps = 1;
  for (Accumulator<float>& angle_accumulator : segment_angle_accumulators_) {
    float abs_step = std::abs(angle_accumulator.last_step());
    if (abs_step > kMaxAngularStep) {
      num_steps = std::max(
          num_steps, static_cast<int>(std::ceil(abs_step / kMaxAngularStep)));
    }
  }

  // Divide the arc into `num_steps`, by interpolating all of the angles
  // together in `num_steps` steps. For each arm segment, generate an
  // `InterpolatorIterator` for the angle of that segment which has
  // `num_steps` subdivisions.
  for (int i = 0; i < kNumSegments; ++i) {
    auto interpolator =
        segment_angle_accumulators_[i].InterpolateLastStepWithStepCount(
            num_steps);
    segment_angle_interpolators_[i] = interpolator;
    segment_angle_iterators_[i] = segment_angle_interpolators_[i].begin();
  }

  flip_oneshot_.Update(state->dt());

  for (int i = 0; i < num_steps; ++i) {
    std::array<float, kNumSegments> segment_angles;
    // For each step, advance all of the iterators one fraction.
    for (int j = 0; j < kNumSegments; ++j) {
      segment_angles[j] = *(segment_angle_iterators_[j]++);
    }

    auto segment_2d =
        ComputeRibbonSegment(state, segment_angles, &debug_segment_points);
    std::pair<glm::vec3, glm::vec3> segment;
    segment.first = glm::vec3(segment_2d.first, 0.1);
    segment.second = glm::vec3(segment_2d.second, 0.1);
    ribbon_.AppendSegment(segment);

    // Mirror around X for the second ribbon.
    segment.first.x *= -1;
    segment.second.x *= -1;

    // If the ribbon is sufficiently close to the horizontal centerline (defined
    // by `y = 0`), and the oneshot timer has elapsed, perform a flip.
    if ((std::abs(segment.first.y) < kFlipTolerance) &&
        (std::abs(segment.second.y) < kFlipTolerance) &&
        flip_oneshot_.IsDue()) {
      flip_y_ = !flip_y_;
      flip_oneshot_.Reset();
    }

    // If the second ribbon has been flipped, do so.
    if (flip_y_) {
      segment.first.y *= -1;
      segment.second.y *= -1;
    }

    // Draw the second ribbon slightly further from the camera.
    segment.first.z = -kEpsilon * 100;
    segment.second.z = -kEpsilon * 100;

    // Flip the direction of the segments so that triangle wrap order is
    // preserved.
    auto temp = segment.first;
    segment.first = segment.second;
    segment.second = temp;
    ribbon2_.AppendSegment(segment);
  }

  float zoom_speed =
      SIGPLOT("zoom_speed",
              SIGINJECT_OVERRIDE("glowsticks_zoom_speed",
                                 1.f + average_power *
                                           (1.1f + sin(state->bass_energy())) *
                                           state->dt() * 10,
                                 0.95f, 1.15f));

  zoom_angle_ += sin(state->bass_energy() * state->dt() * 10) * state->bass() *
                 state->dt() * 10;

  {
    auto front_activation = front_render_target_->Activate();
    ribbon_program_->Use();

    glm::vec3 look_vec_3d(-UnitVectorAtAngle(zoom_angle_) / 2.0f, zoom_speed);
    glm::vec3 axis = glm::cross(glm::vec3(0, 0, 1), look_vec_3d);
    float angle = glm::angle(glm::vec3(0, 0, 1), glm::normalize(look_vec_3d));

    // Draw the waveform.
    ribbon_.UpdateColor(
        HsvToRgb(glm::vec3(energy * color_coefficients_[0], 1, 0.5)));
    ribbon2_.UpdateColor(
        HsvToRgb(glm::vec3(energy * color_coefficients_[1], 1, 0.5)));

    glm::mat4 rotation = glm::rotate(angle, glm::normalize(axis)) *
                         glm::rotate(zoom_angle_ * -2, glm::vec3(0, 0, 1));
    GlBindUniform(ribbon_program_, "model_transform", rotation);
    glEnable(GL_DEPTH_TEST);
    ribbon_.Draw();
    // TODO: Have the second ribbon split off of and rejoin the first ribbon at
    // intervals.
    ribbon2_.Draw();
    glDisable(GL_DEPTH_TEST);
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    GlBindUniform(warp_program_, "last_frame_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(warp_program_, "power", state->bass());
    GlBindUniform(warp_program_, "energy", state->bass_energy());
    GlBindUniform(warp_program_, "zoom_angle", zoom_angle_);
    GlBindUniform(warp_program_, "zoom_speed", zoom_speed);
    GlBindUniform(warp_program_, "framerate_scale", state->dt() * 5);
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));
    GlBindRenderTargetTextureToUniform(
        warp_program_, "last_frame", front_render_target_,
        gl::GlTextureBindingOptions(
            {.sampling_mode = gl::GlTextureSamplingMode::kClampToBorder,
             .border_color = glm::vec4(
                 HsvToRgb({state->bass_energy() *
                               SIGINJECT_OVERRIDE("glowsticks_border_hue_coeff",
                                                  2.0f, 0.0f, 10.0f),
                           1,
                           std::clamp(state->bass() *
                                          SIGINJECT_OVERRIDE(
                                              "glowsticks_border_value_coeff",
                                              2.0f, 0.0f, 10.0f),
                                      0.0f, 1.0f)}),
                 0.5)}));

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();

    GlBindUniform(composite_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(composite_program_, "power", power);
    GlBindUniform(composite_program_, "normalized_energy", normalized_energy);
    GlBindUniform(composite_program_, "alpha", 1.0f);
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());

    const auto square_dimension = std::max(width(), height());
    const int offset_x = -(square_dimension - width()) / 2;
    const int offset_y = -(square_dimension - height()) / 2;
    glViewport(offset_x, offset_y, square_dimension, square_dimension);
    rectangle_.Draw();

    if (kDrawDebugSegments) {
      debug_segments_.UpdateVertices(debug_segment_points);
      debug_segments_.UpdateColor(glm::vec3(1, 1, 1));
      debug_segments_.UpdateWidth(1);
      debug_segments_.Draw();
    }

    back_render_target_->swap_texture_unit(front_render_target_.get());
  }
}

}  // namespace opendrop
