#ifndef OPEN_DROP_CONTROLLER_INTERFACE_H_
#define OPEN_DROP_CONTROLLER_INTERFACE_H_

#include <cstddef>
#include <memory>

#include "absl/types/span.h"
#include "util/audio/audio_processor.h"
#include "util/graphics/gl_interface.h"

namespace opendrop {

class OpenDropControllerInterface {
 public:
  struct Options {
    std::shared_ptr<gl::GlInterface> gl_interface;
    int sampling_rate;
    ptrdiff_t audio_buffer_size;
  };
  // Initializes an OpenDropControllerInterface with the given GlInterface and
  // audio buffer size.
  OpenDropControllerInterface(Options options)
      : options_(std::move(options)),
        audio_processor_(
            std::make_shared<AudioProcessor>(options_.audio_buffer_size)) {}
  virtual ~OpenDropControllerInterface() {}

  // Updates the GL surface. This should be invoked if the output surface
  // changes size.
  virtual void UpdateGeometry(int width, int height) = 0;

  // Draws a single frame.
  virtual void DrawFrame(float dt) = 0;

  // Returns the AudioProcessor associated with this
  // OpenDropControllerInterface.
  AudioProcessor& audio_processor() { return *audio_processor_; }

  std::shared_ptr<gl::GlInterface> gl_interface() const {
    return options_.gl_interface;
  }
  std::shared_ptr<AudioProcessor> audio_processor() const {
    return audio_processor_;
  }

 private:
  const Options options_;

  // The AudioProcessor used by this OpenDropControllerInterface.
  std::shared_ptr<AudioProcessor> audio_processor_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_INTERFACE_H_
