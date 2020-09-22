#include "libopendrop/preset/kaleidoscope/kaleidoscope.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <vector>

#include "libopendrop/preset/kaleidoscope/composite.fsh.h"
#include "libopendrop/preset/kaleidoscope/passthrough.vsh.h"
#include "libopendrop/preset/kaleidoscope/warp.fsh.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.20f;
}

Kaleidoscope::Kaleidoscope(int width, int height) : Preset(width, height) {
  warp_program_ =
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code());
  if (warp_program_ == nullptr) {
    abort();
  }
  composite_program_ =
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), composite_fsh::Code());
  if (composite_program_ == nullptr) {
    abort();
  }

  front_render_target_ = std::make_shared<gl::GlRenderTarget>(width, height, 0);
  back_render_target_ = std::make_shared<gl::GlRenderTarget>(width, height, 1);
}

void Kaleidoscope::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

glm::vec3 HsvToRgb(glm::vec3 hsv) {
  auto normalized_offset_sin = [&](float x, float offset) {
    return (1.0f + sin((x + offset) * M_PI * 2)) / 2;
  };
  return glm::vec3(normalized_offset_sin(hsv.x, 0.0f),
                   normalized_offset_sin(hsv.x, 0.333f),
                   normalized_offset_sin(hsv.x, 0.666f));
}

void Kaleidoscope::OnDrawFrame(absl::Span<const float> samples,
                               std::shared_ptr<GlobalState> state) {
  float energy = state->energy() / 100;
  float power = state->power() / 100;
  float average_power = state->average_power();
  float normalized_power =
      (average_power > 0.0f) ? power / average_power : 0.0f;

  static auto buffer_size = samples.size() / 2;
  static std::vector<glm::vec2> vertices;
  vertices.resize(buffer_size);

  static Rectangle rectangle;
  static Polyline polyline;

  for (int i = 0; i < buffer_size; ++i) {
    float c3 = cos(energy / 10 + power / 100);
    float s3 = sin(energy / 10 + power / 100);
    float x_int = samples[i * 2] * kScaleFactor;
    float y_int = samples[i * 2 + 1] * kScaleFactor;

    float x_pos = x_int * c3 - y_int * s3;
    float y_pos = x_int * s3 + y_int * c3;

    x_pos += cos(sin(2 * energy) * 5 * energy / 1.25 + power / 100) / 5;
    x_pos += cos(sin(2 * energy) * 5 * energy / 5.23 + 0.5) / 20;
    y_pos += sin(sin(2 * energy) * 5 * energy / 1.25 + power / 100) / 5;
    y_pos += sin(sin(2 * energy) * 5 * energy / 5.23 + 0.5) / 20;

    x_pos += 0.5;

    vertices[i] = glm::vec2(x_pos, y_pos);
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    LOG(DEBUG) << "Using program";
    int texture_location =
        glGetUniformLocation(warp_program_->program_handle(), "last_frame");
    LOG(DEBUG) << "Got texture location: " << texture_location;
    int texture_size_location = glGetUniformLocation(
        warp_program_->program_handle(), "last_frame_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    int power_location =
        glGetUniformLocation(warp_program_->program_handle(), "power");
    int energy_location =
        glGetUniformLocation(warp_program_->program_handle(), "energy");
    LOG(DEBUG) << "Got locations for power: " << power_location
               << " and energy: " << energy_location;
    glUniform1f(power_location, power);
    glUniform1f(energy_location, energy);
    LOG(DEBUG) << "Power: " << power << " energy: " << energy;
    glUniform2i(texture_size_location, width(), height());
    int texture_number = 0;
    glActiveTexture(GL_TEXTURE0 + texture_number);
    LOG(DEBUG) << "Configured active texture: " << GL_TEXTURE0 + texture_number;
    glBindTexture(GL_TEXTURE_2D, front_render_target_->texture_handle());
    LOG(DEBUG) << "Bound texture: " << front_render_target_->texture_handle();
    glUniform1i(texture_location, texture_number);
    LOG(DEBUG) << "Configured uniform at location: " << texture_location
               << " to texture index: " << texture_number;

    rectangle.Draw();

    polyline.UpdateVertices(vertices);
    polyline.UpdateWidth(log(normalized_power) * 50);
    polyline.UpdateColor(HsvToRgb(glm::vec3(energy * 100, 1, 0.5)));
    polyline.Draw();

    glFlush();
  }

  composite_program_->Use();
  LOG(DEBUG) << "Using program";
  int texture_location = glGetUniformLocation(
      composite_program_->program_handle(), "render_target");
  LOG(DEBUG) << "Got texture location: " << texture_location;
  int texture_number = 0;
  glActiveTexture(GL_TEXTURE0 + texture_number);
  LOG(DEBUG) << "Configured active texture: " << GL_TEXTURE0 + texture_number;
  glBindTexture(GL_TEXTURE_2D, back_render_target_->texture_handle());
  LOG(DEBUG) << "Bound texture: " << back_render_target_->texture_handle();
  glUniform1i(texture_location, texture_number);
  LOG(DEBUG) << "Configured uniform at location: " << texture_location
             << " to texture index: " << texture_number;
  int texture_size_location = glGetUniformLocation(
      composite_program_->program_handle(), "render_target_size");
  LOG(DEBUG) << "Got texture size location: " << texture_size_location;
  glUniform2i(texture_size_location, width(), height());

  glViewport(0, 0, width(), height());
  rectangle.Draw();

  back_render_target_->swap_texture_unit(front_render_target_.get());
  glFlush();
}

}  // namespace opendrop