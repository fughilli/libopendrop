#ifndef OPEN_DROP_CONTROLLER_INTERFACE_H_
#define OPEN_DROP_CONTROLLER_INTERFACE_H_

#include <memory>

#include "absl/types/span.h"
#include "libopendrop/gl_interface.h"

namespace opendrop {

enum class PcmFormat : int {
  kMono = 0,
  kStereoInterleaved = 1,
};

class OpenDropControllerInterface {
 public:
  // Initializes an OpenDropControllerInterface with the given GlInterface.
  OpenDropControllerInterface(std::shared_ptr<gl::GlInterface> gl_interface)
      : gl_interface_(gl_interface) {}
  virtual ~OpenDropControllerInterface() {}

  // Adds PCM samples to the audio buffer.
  virtual void AddPcmSamples(PcmFormat format,
                             absl::Span<const float> samples) = 0;

  // Updates the GL surface. This should be invoked if the output surface
  // changes size.
  virtual void UpdateGeometry(int width, int height) = 0;

  // Draws a single frame.
  virtual void DrawFrame(float dt) = 0;

 protected:
  std::shared_ptr<gl::GlInterface> gl_interface_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_INTERFACE_H_
