#include "libopendrop/open_drop_controller.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>

#include "libopendrop/util/logging.h"

namespace opendrop {

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface, ptrdiff_t audio_buffer_size,
    int width, int height)
    : OpenDropControllerInterface(gl_interface, audio_buffer_size) {
  UpdateGeometry(height, width);

  global_state_ = std::make_shared<GlobalState>();
}

void OpenDropController::UpdateGeometry(int width, int height) {
  width_ = width;
  height_ = height;
  if (preset_) {
    preset_->UpdateGeometry(width_, height_);
  }
}

void OpenDropController::DrawFrame(float dt) {
  std::vector<float> samples_interleaved;
  samples_interleaved.resize(GetAudioProcessor().buffer_size() *
                             GetAudioProcessor().channels_per_sample());
  if (!GetAudioProcessor().GetSamples(absl::Span<float>(samples_interleaved))) {
    LOG(ERROR) << "Failed to get samples";
    return;
  }

  global_state_->Update(absl::Span<const float>(samples_interleaved), dt);

  if (preset_) {
    preset_->DrawFrame(absl::Span<const float>(samples_interleaved),
                       global_state_);
  }
}

}  // namespace opendrop
