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

namespace {
constexpr float kScaleFactor = 0.3f;

const std::string kVertexShaderCode = R"(
#version 120

varying vec2 screen_uv;

void main(){
    screen_uv = ftransform().xy;
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    gl_BackColor = vec4(1.0 - gl_Color.r, gl_Color.gba);
}
)";
const std::string kFragmentShaderCode = R"(
#version 120

varying vec2 screen_uv;

uniform sampler2D last_frame;
uniform ivec2 last_frame_size;
uniform float energy_time;
uniform float energy;

const bool kScreenToTex = true;
const bool kClamp = true;

vec2 rotate(vec2 screen_uv, float angle) {
  float c = cos(angle);
  float s = sin(angle);

  return vec2(c * screen_uv.x - s * screen_uv.y,
              s * screen_uv.x + c * screen_uv.y);
}

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

vec2 zoom(vec2 screen_uv, float zoom) { return screen_uv * zoom; }

void main() {
  vec2 offset = 1. / last_frame_size;
  float mod_a = sin(energy_time * 0.313 * 3.1415926);
  float mod_b = sin(energy_time * 0.0782 * 3.1415926);
  vec2 texture_uv =
      rotate(zoom(screen_uv, mod_a / 10 * energy + 1.), mod_b * energy);
  if (kScreenToTex) {
    texture_uv = screen_to_tex(texture_uv);
  }

  if (kClamp) {
    texture_uv = clamp(texture_uv, (kScreenToTex ? 0. : -1.), 1.);
  }

  float blur_distance = 2 - energy * 10;

  gl_FragData[0] =
      gl_Color +
      (texture2D(last_frame, texture_uv + vec2(offset.x, 0.) * blur_distance) +
       texture2D(last_frame, texture_uv - vec2(offset.x, 0.) * blur_distance) +
       texture2D(last_frame, texture_uv + vec2(0., offset.y) * blur_distance) +
       texture2D(last_frame, texture_uv - vec2(0., offset.y) * blur_distance)) *
          0.25 *
          (((mod(atan(screen_uv.y, screen_uv.x) + energy_time / 10,
                 3.1415926 / 6)) -
            (3.1415926 / 3)) *
               0.01 +
           1.);
}
)";

const std::string kRenderTextureVertexShaderCode = R"(
#version 120

varying vec2 screen_uv;

void main(){
    screen_uv = ftransform().xy;
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
    gl_BackColor = vec4(1.0 - gl_Color.r, gl_Color.gba);
}
)";

const std::string kRenderTextureFragmentShaderCode = R"(
#version 120

uniform sampler2D render_target;
uniform ivec2 render_target_size;

varying vec2 screen_uv;

vec2 screen_to_tex(vec2 screen_uv) { return (screen_uv + vec2(1., 1.)) * 0.5; }

void main() {
  vec2 tex_uv = screen_to_tex(screen_uv);
  vec2 offset = 1. / render_target_size;
  gl_FragColor = texture2D(render_target, tex_uv) * 1.3 -
                 (texture2D(render_target, tex_uv + vec2(offset.x, 0.)) +
                  texture2D(render_target, tex_uv + vec2(0., offset.y))) *
                     0.4;
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

std::shared_ptr<gl::GlProgram> MakeProgram(std::string vertex_code,
                                           std::string fragment_code) {
  auto gl_program = std::make_shared<gl::GlProgram>();
  gl::GlShader vertex_shader(gl::GlShaderType::kVertex, vertex_code);
  std::string error_string = "";
  if (!vertex_shader.Compile(&error_string)) {
    LOG(ERROR) << "Failed to compile vertex shader: " << error_string;
    return nullptr;
  }
  gl::GlShader fragment_shader(gl::GlShaderType::kFragment, fragment_code);
  if (!fragment_shader.Compile(&error_string)) {
    LOG(ERROR) << "Failed to compile fragment shader: " << error_string;
    return nullptr;
  }

  if (!gl_program->Attach(vertex_shader)
           .Attach(fragment_shader)
           .Link(&error_string)) {
    LOG(ERROR) << "Failed to link program: " << error_string;
    return nullptr;
  }

  return gl_program;
}
}  // namespace

OpenDropController::OpenDropController(
    std::shared_ptr<gl::GlInterface> gl_interface, ptrdiff_t audio_buffer_size,
    int width, int height)
    : OpenDropControllerInterface(gl_interface, audio_buffer_size) {
  UpdateGeometry(height, width);
  compile_context_ = gl_interface_->AllocateSharedContext();

  program_ = MakeProgram(kVertexShaderCode, kFragmentShaderCode);
  if (program_ == nullptr) {
    abort();
  }
  render_target_program_ = MakeProgram(kRenderTextureVertexShaderCode,
                                       kRenderTextureFragmentShaderCode);
  if (render_target_program_ == nullptr) {
    abort();
  }

  render_target_ = std::make_shared<gl::GlRenderTarget>(width, height);
}

void OpenDropController::UpdateGeometry(int width, int height) {
  width_ = width;
  height_ = height;
  glViewport(0, 0, width, height);
  if (render_target_ != nullptr) {
    render_target_->UpdateGeometry(width, height);
  }
}

void OpenDropController::DrawFrame(float dt) {
  static float t = 0;
  static float energy_time = 0;
  t += dt;

  std::vector<float> vertices;
  auto buffer_size = GetAudioProcessor().buffer_size();
  vertices.resize(buffer_size * 3, 0);
  std::vector<float> samples_interleaved;
  samples_interleaved.resize(buffer_size *
                             GetAudioProcessor().channels_per_sample());
  if (!GetAudioProcessor().GetSamples(absl::Span<float>(samples_interleaved))) {
    LOG(ERROR) << "Failed to get samples";
    return;
  }
  static float last_energy = 0;
  float energy = 0;
  for (int i = 0; i < buffer_size; ++i) {
    energy += (pow(samples_interleaved[i * 2], 2) +
               pow(samples_interleaved[i * 2 + 1], 2));
    float c3 = cos(energy_time / 10 + last_energy / 100);
    float s3 = sin(energy_time / 10 + last_energy / 100);
    float x_int = samples_interleaved[i * 2] * kScaleFactor;
    float y_int = samples_interleaved[i * 2 + 1] * kScaleFactor;

    float x_pos = x_int * c3 - y_int * s3;
    float y_pos = x_int * s3 + y_int * c3;

    x_pos += cos(energy_time / 1.25 + last_energy / 100) / 2;
    x_pos += cos(energy_time / 5.23 + 0.5) / 5;
    y_pos += sin(energy_time / 1.25 + last_energy / 100) / 2;
    y_pos += sin(energy_time / 5.23 + 0.5) / 5;
    float z_pos = -0.1;

    vertices[i * 3] = x_pos;
    vertices[i * 3 + 1] = y_pos;
    vertices[i * 3 + 2] = z_pos;
  }
  last_energy = energy;
  energy /= (2 * buffer_size);
  energy_time += energy;

  std::vector<uint16_t> indices;
  indices.resize(buffer_size, 0);

  for (int i = 0; i < buffer_size; ++i) {
    indices[i] = i;
  }

  {
    auto activation = render_target_->Activate();

    glEnableClientState(GL_VERTEX_ARRAY);

    program_->Use();

    LOG(DEBUG) << "Using program";
    int texture_location =
        glGetUniformLocation(program_->program_handle(), "last_frame");
    LOG(DEBUG) << "Got texture location: " << texture_location;
    int texture_size_location =
        glGetUniformLocation(program_->program_handle(), "last_frame_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    int energy_time_location =
        glGetUniformLocation(program_->program_handle(), "energy_time");
    int energy_location =
        glGetUniformLocation(program_->program_handle(), "energy");
    LOG(DEBUG) << "Got locations for energy time: " << energy_time_location
               << " and energy: " << energy_location;
    glUniform1f(energy_time_location, energy_time);
    glUniform1f(energy_location, energy);
    LOG(DEBUG) << "Energy: " << energy << " energy time: " << energy_time;
    glUniform2i(texture_size_location, width_, height_);
    int texture_number = 0;
    glActiveTexture(GL_TEXTURE0 + texture_number);
    LOG(DEBUG) << "Configured active texture: " << GL_TEXTURE0 + texture_number;
    glBindTexture(GL_TEXTURE_2D, render_target_->texture_handle());
    LOG(DEBUG) << "Bound texture: " << render_target_->texture_handle();
    glUniform1i(texture_location, texture_number);
    LOG(DEBUG) << "Configured uniform at location: " << texture_location
               << " to texture index: " << texture_number;

    glColor4f(0., 0., 0., 1.);
    glVertexPointer(3, GL_FLOAT, 0, kFullscreenVertices);
    glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, kFullscreenIndices);

    glLineWidth(4);
    glEnable(GL_LINE_SMOOTH);
    glColor4f(sin(energy_time / 3) / 2 + 0.55, sin(energy_time / 2) / 2 + 0.55,
              sin(energy_time / 5) / 2 + 0.55, 1);
    glVertexPointer(3, GL_FLOAT, 0, vertices.data());
    glDrawElements(GL_LINE_STRIP, buffer_size, GL_UNSIGNED_SHORT,
                   indices.data());
    glDisableClientState(GL_VERTEX_ARRAY);
    glFlush();
  }

  render_target_program_->Use();
  LOG(DEBUG) << "Using program";
  int texture_location = glGetUniformLocation(
      render_target_program_->program_handle(), "render_target");
  LOG(DEBUG) << "Got texture location: " << texture_location;
  int texture_number = 0;
  glActiveTexture(GL_TEXTURE0 + texture_number);
  LOG(DEBUG) << "Configured active texture: " << GL_TEXTURE0 + texture_number;
  glBindTexture(GL_TEXTURE_2D, render_target_->texture_handle());
  LOG(DEBUG) << "Bound texture: " << render_target_->texture_handle();
  glUniform1i(texture_location, texture_number);
  LOG(DEBUG) << "Configured uniform at location: " << texture_location
             << " to texture index: " << texture_number;
  int texture_size_location = glGetUniformLocation(
      render_target_program_->program_handle(), "render_target_size");
  LOG(DEBUG) << "Got texture size location: " << texture_size_location;
  glUniform2i(texture_size_location, width_, height_);

  glViewport(0, 0, width_, height_);
  glEnableClientState(GL_VERTEX_ARRAY);
  glColor4f(0.0, 0.0, 0.0, 1);
  glVertexPointer(3, GL_FLOAT, 0, kFullscreenVertices);
  glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_BYTE, kFullscreenIndices);
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();
}

}  // namespace opendrop
