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

namespace {
// Decay coefficient for the audio sample normalizer. This value should be
// sufficiently close to 1 that quieter parts of songs do not cause the
// normalization factor to decay significantly.
constexpr float kNormalizerAlpha = 0.95f;

// Whether or not to immediately increase the audio normalization coefficient
// when a higher maximum value is detected.
constexpr bool kNormalizerInstantUpscale = true;
}  // namespace

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface,
    std::shared_ptr<gl::GlTextureManager> texture_manager,
    ptrdiff_t audio_buffer_size, int width, int height)
    : OpenDropControllerInterface(gl_interface, audio_buffer_size),
      texture_manager_(texture_manager) {
  UpdateGeometry(height, width);

  global_state_ = std::make_shared<GlobalState>();
  normalizer_ =
      std::make_shared<Normalizer>(kNormalizerAlpha, kNormalizerInstantUpscale);

  auto status_or_render_target =
      gl::GlRenderTarget::MakeShared(width, height, texture_manager_);
  CHECK(status_or_render_target.ok())
      << "Failed to create output render target";
  output_render_target_ = *status_or_render_target;

  absl::StatusOr<std::shared_ptr<gl::GlProgram>> status_or_blit_program =
      gl::GlProgram::MakeShared(blit_vsh::Code(), blit_fsh::Code());
  CHECK(status_or_blit_program.ok()) << "Failed to create blit program";
  blit_program_ = *status_or_blit_program;

  preset_blender_ = std::make_shared<PresetBlender>(width, height);
  CHECK_NULL(preset_blender_) << "Failed to create preset blender";
}

void OpenDropController::UpdateGeometry(int width, int height) {
  width_ = width;
  height_ = height;
  if (preset_blender_) {
    preset_blender_->UpdateGeometry(width_, height_);
  }

  if (output_render_target_) {
    output_render_target_->UpdateGeometry(width_, height_);
  }
}

void OpenDropController::DrawFrame(float dt) {
  static std::vector<float> samples_interleaved;
  samples_interleaved.resize(GetAudioProcessor().buffer_size() *
                             GetAudioProcessor().channels_per_sample());
  if (!GetAudioProcessor().GetSamples(absl::Span<float>(samples_interleaved))) {
    LOG(ERROR) << "Failed to get samples";
    return;
  }

  normalizer_->Normalize(absl::Span<const float>(samples_interleaved), dt,
                         absl::Span<float>(samples_interleaved));
  global_state_->Update(absl::Span<const float>(samples_interleaved), dt);

  if (preset_blender_) {
    preset_blender_->DrawFrame(absl::Span<const float>(samples_interleaved),
                               global_state_, output_render_target_);
    {
      blit_program_->Use();
      gl::GlBindRenderTargetTextureToUniform(blit_program_, "source_texture",
                                             output_render_target_,
                                             gl::GlTextureBindingOptions());
      glUniform1f(
          glGetUniformLocation(blit_program_->program_handle(), "alpha"), 1.0f);

      static Rectangle rectangle;
      rectangle.Draw();
    }
  }
}

}  // namespace opendrop
