#ifndef PRESETS_GLOWSTICKS_3D_GLOWSTICKS_3D_H_
#define PRESETS_GLOWSTICKS_3D_GLOWSTICKS_3D_H_

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "absl/status/statusor.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/gl_texture_manager.h"
#include "libopendrop/preset/preset.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/primitive/ribbon.h"
#include "libopendrop/util/accumulator.h"
#include "libopendrop/util/oneshot.h"

namespace opendrop {

class Glowsticks3d : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "Glowsticks3d"; }

 protected:
  Glowsticks3d(std::shared_ptr<gl::GlProgram> warp_program,
               std::shared_ptr<gl::GlProgram> ribbon_program,
               std::shared_ptr<gl::GlProgram> composite_program,
               std::shared_ptr<gl::GlRenderTarget> front_render_target,
               std::shared_ptr<gl::GlRenderTarget> back_render_target,
               std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  // Number of segments on the armature that describes the motion of the ribbon.
  static constexpr int kNumSegments = 3;

  // Updates the angles of the rotating armatures that describe the motion of
  // the ribbon from the state for the current frame.
  void UpdateArmatureSegmentAngles(
      std::shared_ptr<GlobalState> state,
      std::array<Accumulator<float>, kNumSegments>* segment_angles);

  // Computes a new segment of the ribbon based upon the state for the current
  // frame and the armature segment angles. Outputs a set of debug points into
  // `debug_segment_points` that can be used to render a visualization of the
  // rotating armatures.
  std::pair<glm::vec2, glm::vec2> ComputeRibbonSegment(
      std::shared_ptr<GlobalState> state,
      const std::array<float, kNumSegments> segment_angles,
      std::array<glm::vec2, kNumSegments + 1>* debug_segment_points);

  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> ribbon_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;

  std::array<float, 3> segment_scales_;
  glm::vec2 base_position_;
  std::array<Accumulator<float>, kNumSegments> segment_angle_accumulators_;
  std::array<float, 2> color_coefficients_;
  std::array<float, kNumSegments> direction_reversal_coefficients_;
  std::array<float, kNumSegments> rotational_rate_coefficients_;

  std::array<Interpolator<float>, kNumSegments> segment_angle_interpolators_;
  std::array<InterpolatorIterator<float>, kNumSegments>
      segment_angle_iterators_;

  Rectangle rectangle_;
  Ribbon<glm::vec3> ribbon_;
  Ribbon<glm::vec3> ribbon2_;
  Polyline debug_segments_;
  bool flip_y_;
  OneshotIncremental<float> flip_oneshot_;
};

}  // namespace opendrop

#endif  // PRESETS_GLOWSTICKS_3D_GLOWSTICKS_3D_H_
