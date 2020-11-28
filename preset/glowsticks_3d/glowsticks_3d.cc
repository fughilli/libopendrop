#include "libopendrop/preset/glowsticks_3d/glowsticks_3d.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <vector>

#include "absl/types/span.h"
#include "libopendrop/preset/glowsticks_3d/composite.fsh.h"
#include "libopendrop/preset/glowsticks_3d/passthrough.vsh.h"
#include "libopendrop/preset/glowsticks_3d/ribbon.fsh.h"
#include "libopendrop/preset/glowsticks_3d/warp.fsh.h"
#include "libopendrop/util/coefficients.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"
#include "libopendrop/util/math.h"
#include "libopendrop/util/status_macros.h"

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

constexpr float kFramerateScale = 60.0f / 60.0f;

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

Glowsticks3d::Glowsticks3d(
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
  segment_scales_ = Coefficients::Random<3>(0.1, 0.3);
  segment_scales_[2] = 1.0f;

  auto base_position_array = Coefficients::Random<2>(-0.2f, 0.2f);
  base_position_ = glm::vec2(base_position_array[0], base_position_array[1]);
  color_coefficients_ = Coefficients::Random<2>(1, 20);
  direction_reversal_coefficients_ = Coefficients::Random<kNumSegments>(5, 20);
  rotational_rate_coefficients_ =
      Coefficients::Random<kNumSegments>(0.1f, 0.3f);
}

absl::StatusOr<std::shared_ptr<Preset>> Glowsticks3d::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(
      auto warp_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto ribbon_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), ribbon_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager,
                                                  {.enable_depth = true}));
  return std::shared_ptr<Glowsticks3d>(new Glowsticks3d(
      warp_program, ribbon_program, composite_program, front_render_target,
      back_render_target, texture_manager));
}

void Glowsticks3d::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());

  const auto square_dimension = std::max(width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(square_dimension, square_dimension);
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(square_dimension, square_dimension);
  }
}

void Glowsticks3d::UpdateArmatureSegmentAngles(
    std::shared_ptr<GlobalState> state,
    std::array<Accumulator<float>, kNumSegments>* segment_angles) {
  float average_power = state->average_power();
  float energy = state->energy();
  float power = state->power();

  for (int i = 0; i < segment_angles->size(); ++i) {
    (*segment_angles)[i] +=
        rotational_rate_coefficients_[i] * SafeDivide(power, average_power) *
        sin(energy * direction_reversal_coefficients_[i]) * kFramerateScale;
  }
}

std::pair<glm::vec2, glm::vec2> Glowsticks3d::ComputeRibbonSegment(
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

void Glowsticks3d::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float average_power = state->average_power();
  float energy = state->energy();
  float normalized_energy = state->normalized_energy();
  float power = state->power();

  LOG(DEBUG) << "Input values: average power=" << average_power
             << ", energy=" << energy
             << ", normalized_energy=" << normalized_energy
             << ", power=" << power;

  std::array<glm::vec2, 4> debug_segment_points;

  UpdateArmatureSegmentAngles(state, &segment_angle_accumulators_);
  // Determine how many steps to divide the arc into, by finding the minumum
  // number of subdivisions that achieves a maximum angular step of
  // `kMaxAngluarStep` for the fastest-changing angle.
  std::array<float, 3> angles_;
  for (int i = 0; i < angles_.size(); ++i) {
    angles_[i] = segment_angle_accumulators_[i];
  }
  LOG(DEBUG) << "segment_angle_accumulators_=" << absl::Span<float>(angles_);
  int num_steps = 1;
  for (Accumulator<float>& angle_accumulator : segment_angle_accumulators_) {
    float abs_step = std::abs(angle_accumulator.last_step());
    if (abs_step > kMaxAngularStep) {
      num_steps = std::max(
          num_steps, static_cast<int>(std::ceil(abs_step / kMaxAngularStep)));
    }
  }

  if (num_steps > 1) {
    LOG(DEBUG) << "num_steps = " << num_steps;
  }

  // Divide the arc into `num_steps`, by interpolating all of the angles
  // together in `num_steps` steps. For each arm segment, generate an
  // `InterpolatorIterator` for the angle of that segment which has
  // `num_steps` subdivisions.
  for (int i = 0; i < kNumSegments; ++i) {
    auto interpolator =
        segment_angle_accumulators_[i].InterpolateLastStepWithStepCount(
            num_steps);
    LOG(DEBUG) << "Interpolating from " << interpolator.begin_value() << " to "
               << interpolator.end_value() << " in " << num_steps << " steps";
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

    LOG(DEBUG) << "Interpolation step " << i << ": "
               << absl::Span<float>(segment_angles);
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
    segment.first.z = -0.1;
    segment.second.z = -0.1;

    // Flip the direction of the segments so that triangle wrap order is
    // preserved.
    auto temp = segment.first;
    segment.first = segment.second;
    segment.second = temp;
    ribbon2_.AppendSegment(segment);
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    LOG(DEBUG) << "Using program";
    int texture_size_location = glGetUniformLocation(
        warp_program_->program_handle(), "last_frame_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    int power_location =
        glGetUniformLocation(warp_program_->program_handle(), "power");
    int energy_location =
        glGetUniformLocation(warp_program_->program_handle(), "energy");
    LOG(DEBUG) << "Got locations for power: " << power_location
               << " and energy: " << energy_location;
    glUniform1f(power_location, power);
    glUniform1f(energy_location, energy);
    LOG(DEBUG) << "Power: " << power << " energy: " << energy;
    glUniform2i(texture_size_location, front_render_target_->width(),
                front_render_target_->height());
    GlBindRenderTargetTextureToUniform(
        warp_program_, "last_frame", front_render_target_,
        gl::GlTextureBindingOptions(
            {.sampling_mode = gl::GlTextureSamplingMode::kClampToBorder,
             .border_color = glm::vec4(0, 0, 0, 1)}));
    glUniform1f(glGetUniformLocation(warp_program_->program_handle(),
                                     "framerate_scale"),
                kFramerateScale);

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();

    // Draw the waveform.
    ribbon_.UpdateColor(
        HsvToRgb(glm::vec3(energy * color_coefficients_[0], 1, 0.5)));
    ribbon2_.UpdateColor(
        HsvToRgb(glm::vec3(energy * color_coefficients_[1], 1, 0.5)));

    glEnable(GL_DEPTH_TEST);
    ribbon_.Draw();
    // TODO: Have the second ribbon split off of and rejoin the first ribbon at
    // intervals.
    ribbon2_.Draw();
    glEnable(GL_DEPTH_TEST);

    glFlush();
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();
    LOG(DEBUG) << "Using program";
    int texture_size_location = glGetUniformLocation(
        composite_program_->program_handle(), "render_target_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    glUniform2i(texture_size_location, back_render_target_->width(),
                back_render_target_->height());
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());
    glUniform1f(
        glGetUniformLocation(composite_program_->program_handle(), "alpha"),
        alpha);
    glUniform1f(
        glGetUniformLocation(composite_program_->program_handle(), "power"),
        power);
    glUniform1f(glGetUniformLocation(composite_program_->program_handle(),
                                     "normalized_energy"),
                normalized_energy);

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
    glFlush();
  }
}

}  // namespace opendrop
