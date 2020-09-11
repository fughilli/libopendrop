#include "libopendrop/open_drop_controller.h"

namespace opendrop {

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface)
    : OpenDropControllerInterface(gl_interface) {
  main_context_ = gl_interface_->AllocateSharedContext();
  compile_context_ = gl_interface_->AllocateSharedContext();
}

void OpenDropController::AddPcmSamples(PcmFormat format,
                                       absl::Span<const float> samples) {}

void OpenDropController::UpdateGeometry() {}

void OpenDropController::DrawFrame() {}

}  // namespace opendrop
