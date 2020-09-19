#ifndef PRESET_H_
#define PRESET_H_

#include <memory>
#include <mutex>

#include "absl/types/span.h"
#include "libopendrop/global_state.h"

namespace opendrop {

// Base class interface for a preset. This class is thread-safe.
class Preset {
 public:
  // Constructs a preset which renders to a raster of the given dimensions.
  Preset(int width, int height) : width_(width), height_(height) {}
  virtual ~Preset() {}

  // Draws a single frame of this preset. `samples` is a buffer of interleaved
  // audio samples. `state` is the current global libopendrop state.
  void DrawFrame(absl::Span<const float> samples,
                 std::shared_ptr<GlobalState> state);

  // Updates the preset render geometry. Subsequent calls to `DrawFrame` will
  // render at these dimensions.
  void UpdateGeometry(int width, int height);

 protected:
  // Getters for dimensions.
  int width() const { return width_; }
  int height() const { return height_; }

  // Callbacks for subclass implementations.
  // Invoked by `DrawFrame` with lock held.
  virtual void OnDrawFrame(absl::Span<const float> samples,
                           std::shared_ptr<GlobalState> state) = 0;
  // Invoked by `UpdateGeometry` with lock held.
  virtual void OnUpdateGeometry() = 0;

 private:
  // Mutex protecting preset state.
  std::mutex state_mu_;
  // Preset render dimensions.
  int width_, height_;
};

}  // namespace opendrop

#endif  // PRESET_H_
