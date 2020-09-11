#include "libopendrop/open_drop_controller.h"

#include <GL/gl.h>
#include <GL/glext.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>
#include <iostream>

namespace opendrop {

namespace {
constexpr ptrdiff_t kSampleBufferNumSamples = 256;
constexpr int kChannelsPerSample = 2;
constexpr float kScaleFactor = 5.0f;

const std::string kVertexShaderCode = R"(
#version 120
attribute vec3 vertex_position;
attribute vec4 vertex_color;
attribute vec2 vertex_texture;
attribute vec2 vertex_rad_ang;

void main(){
    vec4 position = vec4(vertex_position, 1.0);
    gl_Position = position;
    gl_FrontColor = gl_Color;
    gl_BackColor = vec4(1.0 - gl_Color.r, gl_Color.gba);
}
)";
const std::string kFragmentShaderCode = R"(
#version 120

attribute vec4 fragment_color;
attribute vec2 fragment_texture;
uniform float t;

void main() {
  gl_FragColor = gl_Color; //vec4(1., fragment_texture, 1.);
}
)";

constexpr float kFullscreenVertices[] = {
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
  samples_interleaved_.resize(kSampleBufferNumSamples * kChannelsPerSample, 0);
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
                                       absl::Span<const float> samples) {
  if (format == PcmFormat::kMono) {
    std::vector<float> intermediate_buffer;
    intermediate_buffer.resize(samples.size() * 2);
    for (int i = 0; i < samples.size(); ++i) {
      intermediate_buffer[i * 2] = samples[i];
      intermediate_buffer[i * 2 + 1] = samples[i];
    }
    AddPcmSamples(PcmFormat::kStereoInterleaved,
                  absl::Span<const float>(intermediate_buffer));
    return;
  }

  std::unique_lock<std::mutex> samples_lock(samples_interleaved_mu_);

  if (samples.size() >= samples_interleaved_.size()) {
    std::copy(samples.begin() + (samples.size() - samples_interleaved_.size()),
              samples.end(), samples_interleaved_.begin());
    return;
  }

  // Move samples forward in buffer
  std::copy(samples_interleaved_.begin() + samples.size(),
            samples_interleaved_.end(), samples_interleaved_.begin());
  // Append new samples to end of old samples
  std::copy(samples.begin(), samples.end(),
            samples_interleaved_.begin() +
                (samples_interleaved_.size() - samples.size()));
}

void OpenDropController::UpdateGeometry(int width, int height) {
  width_ = width;
  height_ = height;
  glViewport(0, 0, width, height);
}

void OpenDropController::DrawFrame(float dt) {
  static float t = 0;
  t += dt;

  glClearColor(0, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  std::vector<float> vertices;
  vertices.resize(kSampleBufferNumSamples * 3, 0);
  {
    std::unique_lock<std::mutex> samples_lock(samples_interleaved_mu_);
    for (int i = 0; i < kSampleBufferNumSamples; ++i) {
      float x_pos = samples_interleaved_[i * 2] * kScaleFactor;
      float y_pos = samples_interleaved_[i * 2 + 1] * kScaleFactor;
      float z_pos = -0.1;

      vertices[i * 3] = x_pos;
      vertices[i * 3 + 1] = y_pos;
      vertices[i * 3 + 2] = z_pos;
    }
  }
  std::vector<uint16_t> indices;
  indices.resize(kSampleBufferNumSamples, 0);

  for (int i = 0; i < kSampleBufferNumSamples; ++i) {
    indices[i] = i;
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  program_->Use();

  glColor4f(0.1, 0.1, 0.1, 1);
  glVertexPointer(3, GL_FLOAT, 0, kFullscreenVertices);
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, kFullscreenIndices);

  glLineWidth(2);
  glEnable(GL_LINE_SMOOTH);
  glColor4f(0, 1, 0, 1);
  glVertexPointer(3, GL_FLOAT, 0, vertices.data());
  glDrawElements(GL_LINE_STRIP, kSampleBufferNumSamples, GL_UNSIGNED_SHORT,
                 indices.data());
  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();
}

}  // namespace opendrop
