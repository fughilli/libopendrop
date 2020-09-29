#ifndef OPEN_DROP_CONTROLLER_H_
#define OPEN_DROP_CONTROLLER_H_

#include <memory>

#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/global_state.h"
#include "libopendrop/normalizer.h"
#include "libopendrop/open_drop_controller_interface.h"
#include "libopendrop/preset/preset.h"

namespace opendrop {

class OpenDropController : public OpenDropControllerInterface {
 public:
  OpenDropController(std::shared_ptr<gl::GlInterface> gl_interface,
                     ptrdiff_t audio_buffer_size, int width, int height);
  void UpdateGeometry(int width, int height) override;
  void DrawFrame(float dt) override;
  void SetPreset(std::shared_ptr<Preset> preset) override { preset_ = preset; }

 protected:
  int width_, height_;
  std::shared_ptr<Preset> preset_;
  std::shared_ptr<GlobalState> global_state_;
  std::shared_ptr<Normalizer> normalizer_;
  std::shared_ptr<gl::GlRenderTarget> output_render_target_;
  std::shared_ptr<gl::GlProgram> blit_program_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_H_
