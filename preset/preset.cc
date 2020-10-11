#include "libopendrop/preset/preset.h"

namespace opendrop {

void Preset::DrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  std::unique_lock<std::mutex> lock(state_mu_);
  if (width_ == 0 || height_ == 0) {
    // Don't draw.
    return;
  }
  OnDrawFrame(samples, state, alpha, output_render_target);
}

void Preset::UpdateGeometry(int width, int height) {
  std::unique_lock<std::mutex> lock(state_mu_);
  width_ = width;
  height_ = height;
  OnUpdateGeometry();
}

}  // namespace opendrop
