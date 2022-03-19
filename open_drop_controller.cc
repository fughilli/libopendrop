#include "open_drop_controller.h"

#include <cmath>
#include <cstdint>
#include <iostream>

#include "blit.fsh.h"
#include "blit.vsh.h"
#include "primitive/rectangle.h"
#include "util/gl_helper.h"
#include "util/gl_util.h"
#include "util/logging.h"

namespace opendrop {

namespace {
// Decay coefficient for the audio sample normalizer. This value should be
// sufficiently close to 1 that quieter parts of songs do not cause the
// normalization factor to decay significantly.
constexpr float kNormalizerAlpha = 0.99f;

// Whether or not to immediately increase the audio normalization coefficient
// when a higher maximum value is detected.
constexpr bool kNormalizerInstantUpscale = true;
}  // namespace

OpenDropController::OpenDropController(Options options)
    : OpenDropControllerInterface(
          {.gl_interface = options.gl_interface,
           .sampling_rate = options.sampling_rate,
           .audio_buffer_size = options.audio_buffer_size}),
      options_(std::move(options)) {
  UpdateGeometry(options_.width, options_.height);

  global_state_ = std::make_shared<GlobalState>(
      GlobalState::Options{.sampling_rate = options_.sampling_rate});
  normalizer_ =
      std::make_shared<Normalizer>(kNormalizerAlpha, kNormalizerInstantUpscale);

  auto status_or_render_target = gl::GlRenderTarget::MakeShared(
      options_.width, options_.height, options_.texture_manager);
  CHECK(status_or_render_target.ok())
      << "Failed to create output render target";
  output_render_target_ = *status_or_render_target;

  absl::StatusOr<std::shared_ptr<gl::GlProgram>> status_or_blit_program =
      gl::GlProgram::MakeShared(blit_vsh::Code(), blit_fsh::Code());
  CHECK(status_or_blit_program.ok()) << "Failed to create blit program";
  blit_program_ = *status_or_blit_program;

  preset_blender_ =
      std::make_shared<PresetBlender>(options_.width, options_.height);
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
  samples_interleaved_.resize(0x10000 * audio_processor().channels_per_sample());
  auto samples_view = absl::Span<float>(samples_interleaved_);
  if (!audio_processor().GetSamples(samples_view)) {
    LOG(ERROR) << "Failed to get samples";
    return;
  }

  samples_view_ = samples_view;

  normalizer_->Normalize(samples_view, dt, samples_view);
  global_state_->Update(samples_view, dt);

  if (preset_blender_) {
    preset_blender_->DrawFrame(samples_view, global_state_,
                               output_render_target_);
    if (options_.draw_output_to_quad) {
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
