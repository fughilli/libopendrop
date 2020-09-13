#ifndef OPEN_DROP_CONTROLLER_H_
#define OPEN_DROP_CONTROLLER_H_

#include <memory>

#include "libopendrop/gl_interface.h"
#include "libopendrop/open_drop_controller_interface.h"

namespace opendrop {

class OpenDropController : public OpenDropControllerInterface {
 public:
  OpenDropController(std::shared_ptr<gl::GlInterface> gl_interface,
                     ptrdiff_t audio_buffer_size, int width, int height);
  void UpdateGeometry(int width, int height) override;
  void DrawFrame(float dt) override;

 protected:
  int width_, height_;
  std::shared_ptr<gl::GlContext> compile_context_;
  std::shared_ptr<gl::GlProgram> program_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_H_
