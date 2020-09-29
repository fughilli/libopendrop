#include "libopendrop/open_drop_controller.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>

#include "libopendrop/blit.fsh.h"
#include "libopendrop/blit.vsh.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface, ptrdiff_t audio_buffer_size,
    int width, int height)
    : OpenDropControllerInterface(gl_interface, audio_buffer_size) {
  UpdateGeometry(height, width);

  global_state_ = std::make_shared<GlobalState>();

  output_render_target_ =
      std::make_shared<gl::GlRenderTarget>(width, height, 2);
  CHECK_NULL(output_render_target_) << "Failed to create output render target";

  blit_program_ = gl::GlProgram::MakeShared(blit_vsh::Code(), blit_fsh::Code());
  CHECK_NULL(blit_program_) << "Failed to create blit program";
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
                       global_state_, output_render_target_);

    {
      blit_program_->Use();
      gl::GlBindRenderTargetTextureToUniform(blit_program_, "source_texture",
                                             output_render_target_);

      static Rectangle rectangle;
      rectangle.Draw();
    }
  }
}

}  // namespace opendrop
