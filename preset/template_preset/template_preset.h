#ifndef PRESETS_TEMPLATE_PRESET_TEMPLATE_PRESET_H_
#define PRESETS_TEMPLATE_PRESET_TEMPLATE_PRESET_H_

#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/preset/preset.h"

namespace opendrop {

class TemplatePreset : public Preset {
 public:
  TemplatePreset(int width, int height);

 protected:
  void OnDrawFrame(absl::Span<const float> samples,
                   std::shared_ptr<GlobalState> state) override;
  void OnUpdateGeometry() override;

 private:
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;
  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
};

}  // namespace opendrop

#endif  // PRESETS_TEMPLATE_PRESET_TEMPLATE_PRESET_H_
