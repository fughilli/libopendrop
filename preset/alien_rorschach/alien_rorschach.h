#ifndef PRESETS_ALIEN_RORSCHACH_ALIEN_RORSCHACH_H_
#define PRESETS_ALIEN_RORSCHACH_ALIEN_RORSCHACH_H_

#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/gl_texture_manager.h"
#include "libopendrop/preset/preset.h"

namespace opendrop {

class AlienRorschach : public Preset {
 public:
  AlienRorschach(std::shared_ptr<gl::GlTextureManager> texture_manager,
                 int width, int height);

 protected:
  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;
  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
};

}  // namespace opendrop

#endif  // PRESETS_ALIEN_RORSCHACH_ALIEN_RORSCHACH_H_