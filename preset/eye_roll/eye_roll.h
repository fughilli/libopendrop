#ifndef PRESET_EYE_ROLL_EYE_ROLL_H_
#define PRESET_EYE_ROLL_EYE_ROLL_H_

#include <vector>

#include "absl/status/statusor.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "preset/preset.h"
#include "primitive/ngon.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "util/signal/filter.h"
#include "third_party/glm_helper.h"
#include "util/signal/signals.h"

namespace opendrop {

class EyeRoll : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "EyeRoll"; }

  int max_count() const override { return 1; }
  bool should_solo() const override { return true; }

 protected:
  EyeRoll(std::shared_ptr<gl::GlProgram> warp_program,
          std::shared_ptr<gl::GlProgram> composite_program,
          std::shared_ptr<gl::GlProgram> ngon_program,
          std::shared_ptr<gl::GlProgram> line_program,
          std::shared_ptr<gl::GlRenderTarget> front_render_target,
          std::shared_ptr<gl::GlRenderTarget> back_render_target,
          std::shared_ptr<gl::GlTextureManager> texture_manager, int n);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  void DrawEye(glm::vec2 center, float scale, float scale_coeff, float angle,
               float eyelid_pos);

  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlProgram> ngon_program_;
  std::shared_ptr<gl::GlProgram> line_program_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;

  Rectangle rectangle_;
  Ngon ngon_;
  Polyline polyline_;
  std::vector<glm::vec2> line_points_;
  float rotary_velocity_l_ = 0;
  float rotary_velocity_r_ = 0;
  float eye_angle_l_ = 0;
  float eye_angle_r_ = 0;

  glm::vec2 velocity_{0, 0};
  glm::vec2 position_{0, 0};

  float line_energy_ = 0;

  RandomEvent blink_event_;
  RandomEvent wink_event_;
  RampTweener left_eye_tweener_;
  RampTweener right_eye_tweener_;

  std::shared_ptr<IirFilter> bass_filter_;
  std::shared_ptr<HystereticMapFilter> bass_power_filter_;
  std::shared_ptr<IirFilter> treble_filter_;
  std::shared_ptr<HystereticMapFilter> treble_power_filter_;
};

}  // namespace opendrop

#endif  // PRESET_EYE_ROLL_EYE_ROLL_H_
