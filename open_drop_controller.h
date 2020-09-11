#ifndef OPEN_DROP_CONTROLLER_H_
#define OPEN_DROP_CONTROLLER_H_

#include "absl/types/span.h"
#include "libopendrop/open_drop_controller_interface.h"

namespace opendrop {

class OpenDropController : public OpenDropControllerInterface {
 public:
  OpenDropController(std::shared_ptr<gl::GlInterface> gl_interface);
  void AddPcmSamples(PcmFormat format,
                     absl::Span<const float> samples) override;

  void UpdateGeometry() override;
  void DrawFrame() override;

 protected:
  std::shared_ptr<gl::GlContext> main_context_;
  std::shared_ptr<gl::GlContext> compile_context_;
};

}  // namespace opendrop

#endif  // OPEN_DROP_CONTROLLER_H_
