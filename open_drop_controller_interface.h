#ifndef OPEN_DROP_CONTROLLER_INTERFACE_H_
#define OPEN_DROP_CONTROLLER_INTERFACE_H_

#include <cstddef>
#include <memory>

#include "absl/types/span.h"
#include "libopendrop/audio_processor.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/preset/preset.h"

namespace opendrop {

class OpenDropControllerInterface {
 public:
  // Initializes an OpenDropControllerInterface with the given GlInterface and
  // audio buffer size.
  OpenDropControllerInterface(std::shared_ptr<gl::GlInterface> gl_interface,
                              ptrdiff_t audio_buffer_size)
      : gl_interface_(gl_interface) {
    audio_processor_ = std::make_shared<AudioProcessor>(audio_buffer_size);
  }
  virtual ~OpenDropControllerInterface() {}

  // Updates the GL surface. This should be invoked if the output surface
  // changes size.
  virtual void UpdateGeometry(int width, int height) = 0;

  // Draws a single frame.
  virtual void DrawFrame(float dt) = 0;

  // Sets the current preset.
  virtual void SetPreset(std::shared_ptr<Preset> preset) = 0;

  // Returns the AudioProcessor associated with this
  // OpenDropControllerInterface.
  AudioProcessor& GetAudioProcessor() { return *audio_processor_; }

 protected:
  // The AudioProcessor used by this OpenDropControllerInterface.
  std::shared_ptr<AudioProcessor> audio_processor_;

  // The GlInterface used by this OpenDropControllerInterface.
  std::shared_ptr<gl::GlInterface> gl_interface_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_INTERFACE_H_
