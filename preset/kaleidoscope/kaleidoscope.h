#ifndef PRESET_KALEIDOSCOPE_KALEIDOSCOPE_H_
#define PRESET_KALEIDOSCOPE_KALEIDOSCOPE_H_

#include <vector>

#include "absl/status/statusor.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "preset/preset.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"

namespace opendrop {

class Kaleidoscope : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "Kaleidoscope"; }

 protected:
  Kaleidoscope(std::shared_ptr<gl::GlProgram> waveform_program,
               std::shared_ptr<gl::GlProgram> warp_program,
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
  std::shared_ptr<gl::GlProgram> waveform_program_;
  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;

  std::vector<glm::vec2> vertices_;
  Rectangle rectangle_;
  Polyline polyline_;
};

}  // namespace opendrop

#endif  // PRESET_KALEIDOSCOPE_KALEIDOSCOPE_H_
