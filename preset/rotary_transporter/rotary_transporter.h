#ifndef PRESETS_ROTARY_TRANSPORTER_ROTARY_TRANSPORTER_H_
#define PRESETS_ROTARY_TRANSPORTER_ROTARY_TRANSPORTER_H_

#include <glm/vec2.hpp>
#include <vector>

#include "absl/status/statusor.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/gl_texture_manager.h"
#include "libopendrop/preset/preset.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/filter.h"

namespace opendrop {

class RotaryTransporter : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "RotaryTransporter"; }

 protected:
  RotaryTransporter(std::shared_ptr<gl::GlProgram> warp_program,
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
  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;

  std::vector<glm::vec2> vertices_;
  Rectangle rectangle_;
  Polyline polyline_;

  float zoom_angle_ = 0.0f;
  std::shared_ptr<IirFilter> vocal_filter_;
  std::shared_ptr<IirFilter> left_vocal_filter_;
  std::shared_ptr<IirFilter> right_vocal_filter_;
  std::shared_ptr<IirFilter> bass_filter_;

  float bass_power_ = 0.0f;
  float bass_energy_ = 0.0f;

  float border_color_phase_ = 0.0f;
};

}  // namespace opendrop

#endif  // PRESETS_ROTARY_TRANSPORTER_ROTARY_TRANSPORTER_H_
