#ifndef APPLICATION_OPEN_DROP_CONTROLLER_H_
#define APPLICATION_OPEN_DROP_CONTROLLER_H_

#include <memory>

#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "application/global_state.h"
#include "util/audio/normalizer.h"
#include "application/open_drop_controller_interface.h"
#include "preset/preset.h"
#include "preset/preset_blender.h"

namespace opendrop {

class OpenDropController : public OpenDropControllerInterface {
 public:
  struct Options {
    std::shared_ptr<gl::GlInterface> gl_interface;
    std::shared_ptr<gl::GlTextureManager> texture_manager;
    int sampling_rate;
    ptrdiff_t audio_buffer_size;
    int width;
    int height;
    bool draw_output_to_quad;
  };

  OpenDropController(Options options);
  void UpdateGeometry(int width, int height) override;
  void DrawFrame(float dt) override;

  // Returns the `PresetBlender` associated with this `OpenDropController`
  // instance.
  std::shared_ptr<PresetBlender> preset_blender() { return preset_blender_; }
  GlobalState& global_state() const { return *global_state_; }
  std::shared_ptr<gl::GlRenderTarget> render_target() {
    return output_render_target_;
  }

  absl::Span<const float> GetCurrentFrameSamples() const {
    return samples_view_;
  }

  int width() const { return width_; }
  int height() const { return height_; }

 private:
  const Options options_;

  int width_, height_;

  std::shared_ptr<PresetBlender> preset_blender_;
  std::shared_ptr<GlobalState> global_state_;
  std::shared_ptr<Normalizer> normalizer_;
  std::shared_ptr<gl::GlRenderTarget> output_render_target_;
  std::shared_ptr<gl::GlProgram> blit_program_;

  std::vector<float> samples_interleaved_;
  absl::Span<const float> samples_view_{};
};

}  // namespace opendrop

#endif  // APPLICATION_OPEN_DROP_CONTROLLER_H_
