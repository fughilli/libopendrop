#ifndef PRESET_H_
#define PRESET_H_

#include <memory>
#include <mutex>

#include "absl/types/span.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/global_state.h"

namespace opendrop {

// Base class interface for a preset. This class is thread-safe.
class Preset {
 public:
  // Constructs a preset which renders to a raster of the given dimensions.
  Preset(std::shared_ptr<gl::GlTextureManager> texture_manager)
      : texture_manager_(texture_manager), width_(0), height_(0) {}
  virtual ~Preset() {}

  // Draws a single frame of this preset. `samples` is a buffer of interleaved
  // audio samples. `state` is the current global libopendrop state. `alpha` is
  // the alpha that should be premultiplied when rendering the output of the
  // preset. `output_render_target` is the render target to render the preset
  // output to.
  void DrawFrame(absl::Span<const float> samples,
                 std::shared_ptr<GlobalState> state, float alpha,
                 std::shared_ptr<gl::GlRenderTarget> output_render_target);

  // Updates the preset render geometry. Subsequent calls to `DrawFrame` will
  // render at these dimensions.
  void UpdateGeometry(int width, int height);

  virtual std::string name() const = 0;

 protected:
  // Getters for dimensions.
  int width() const { return width_; }
  int height() const { return height_; }

  // Callbacks for subclass implementations.
  // Invoked by `DrawFrame` with lock held.
  virtual void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) = 0;
  // Invoked by `UpdateGeometry` with lock held.
  virtual void OnUpdateGeometry() = 0;

  // Getter for texture manager.
  std::shared_ptr<gl::GlTextureManager> texture_manager() {
    return texture_manager_;
  }

 private:
  // the GlTextureManager providing texture units for this preset.
  std::shared_ptr<gl::GlTextureManager> texture_manager_;
  // Mutex protecting preset state.
  std::mutex state_mu_;
  // Preset render dimensions.
  int width_, height_;
};

}  // namespace opendrop

#endif  // PRESET_H_
