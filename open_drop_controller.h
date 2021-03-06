#ifndef OPEN_DROP_CONTROLLER_H_
#define OPEN_DROP_CONTROLLER_H_

#include <memory>

#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/global_state.h"
#include "libopendrop/normalizer.h"
#include "libopendrop/open_drop_controller_interface.h"
#include "libopendrop/preset/preset.h"
#include "libopendrop/preset/preset_blender.h"

namespace opendrop {

class OpenDropController : public OpenDropControllerInterface {
 public:
  OpenDropController(std::shared_ptr<gl::GlInterface> gl_interface,
                     std::shared_ptr<gl::GlTextureManager> texture_manager,
                     ptrdiff_t audio_buffer_size, int width, int height);
  void UpdateGeometry(int width, int height) override;
  void DrawFrame(float dt) override;

  // Returns the `PresetBlender` associated with this `OpenDropController`
  // instance.
  std::shared_ptr<PresetBlender> preset_blender() { return preset_blender_; }
  GlobalState& global_state() const { return *global_state_; }

 protected:
  std::shared_ptr<gl::GlTextureManager> texture_manager_;
  int width_, height_;
  std::shared_ptr<PresetBlender> preset_blender_;
  std::shared_ptr<GlobalState> global_state_;
  std::shared_ptr<Normalizer> normalizer_;
  std::shared_ptr<gl::GlRenderTarget> output_render_target_;
  std::shared_ptr<gl::GlProgram> blit_program_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_H_
