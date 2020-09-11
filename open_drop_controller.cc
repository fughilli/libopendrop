#include "libopendrop/open_drop_controller.h"

#include <GL/gl.h>
#include <GL/glext.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>

namespace opendrop {

namespace {
const std::string kVertexShaderCode = R"(
#version 120
attribute vec3 vertex_position;
attribute vec4 vertex_color;
attribute vec2 vertex_texture;
attribute vec2 vertex_rad_ang;

void main(){
    vec4 position = vec4(vertex_position, 1.0);
    gl_Position = position;
}
)";
const std::string kFragmentShaderCode = R"(
#version 120

varying vec4 fragment_color;
uniform float t;

void main() {
  gl_FragColor = vec4(1., 1., 0., 1.);
}
)";

constexpr GLfloat kFullscreenVertices[] = {
    -1, -1, 0,  // Lower-left
    -1, 1,  0,  // Upper-left
    1,  1,  0,  // Upper-right
    1,  -1, 0,  // Lower-right
};

constexpr uint8_t kFullscreenIndices[] = {
    0,
    1,
    2,
    3,
};
}  // namespace

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface, int height, int width)
    : OpenDropControllerInterface(gl_interface) {
  UpdateGeometry(height, width);
  compile_context_ = gl_interface_->AllocateSharedContext();

  program_ = std::make_shared<gl::GlProgram>();

  gl::GlShader vertex_shader(gl::GlShaderType::kVertex, kVertexShaderCode);
  gl::GlShader fragment_shader(gl::GlShaderType::kFragment,
                               kFragmentShaderCode);
  std::string error_string = "";
  if (!fragment_shader.Compile(&error_string)) {
    std::cerr << "Failed to compile fragment shader: " << error_string
              << std::endl;
    return;
  }

  if (!program_->Attach(vertex_shader)
           .Attach(fragment_shader)
           .Link(&error_string)) {
    std::cerr << "Failed to link program: " << error_string << std::endl;
  }
}

void OpenDropController::AddPcmSamples(PcmFormat format,
                                       absl::Span<const float> samples) {}

void OpenDropController::UpdateGeometry(int width, int height) {
  width_ = width;
  height_ = height;
  glViewport(0, 0, width, height);
}

void OpenDropController::DrawFrame(float dt) {
  static float t = 0;
  t += dt;

  glClearColor(0, 1, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  program_->Use();

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, kFullscreenVertices);
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, kFullscreenIndices);
  glDisableClientState(GL_VERTEX_ARRAY);

  glFlush();
}

}  // namespace opendrop
