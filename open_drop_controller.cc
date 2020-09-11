#include "libopendrop/open_drop_controller.h"

#include <GL/gl.h>
#include <GL/glext.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

namespace opendrop {

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface, int height, int width)
    : OpenDropControllerInterface(gl_interface) {
  UpdateGeometry(height, width);
  compile_context_ = gl_interface_->AllocateSharedContext();
}

void OpenDropController::AddPcmSamples(PcmFormat format,
                                       absl::Span<const float> samples) {}

void OpenDropController::UpdateGeometry(int height, int width) {
  height_ = height;
  width_ = width;
}

void OpenDropController::DrawFrame(float dt) {
  static float t = 0;
  t += dt;
  float color_value_r = (std::sin(t * M_PI) + 1) / 2;
  float color_value_g = (std::sin((t + 0.33) * M_PI) + 1) / 2;
  float color_value_b = (std::sin((t + 0.66) * M_PI) + 1) / 2;
  glClearColor(color_value_r, color_value_g, color_value_b, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

}  // namespace opendrop
