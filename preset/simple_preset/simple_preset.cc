#include "libopendrop/preset/simple_preset/simple_preset.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <vector>

#include "libopendrop/preset/simple_preset/composite.fsh.h"
#include "libopendrop/preset/simple_preset/passthrough.vsh.h"
#include "libopendrop/preset/simple_preset/warp.fsh.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.3f;

const std::vector<std::vector<glm::vec2>> kLetters = {
    {{-1, -1}, {-1, 1}, {-1, 0}, {1, 0}, {1, -1}, {1, 1}},           // H
    {{-1, -1}, {0, 1}, {1, -1}, {0.5, 0}, {-0.5, 0}},                // A
    {{-1, -1}, {-1, 1}, {1, 1}, {1, 0}, {-1, 0}},                    // P
    {{-1, -1}, {-1, 1}, {1, 1}, {1, 0}, {-1, 0}},                    // P
    {{0, -1}, {0, 0}, {-1, 1}, {0, 0}, {1, 1}},                      // Y
    {},                                                              // <space>
    {{-1, -1}, {-1, 1}, {1, 1}, {-1, 0}, {1, -1}, {-1, -1}},         // B
    {{0, -1}, {0, 1}},                                               // I
    {{-1, -1}, {-1, 1}, {1, 1}, {1, 0}, {-1, 0}, {1, -1}},           // R
    {{0, -1}, {0, 1}, {-1, 1}, {1, 1}},                              // T
    {{-1, -1}, {-1, 1}, {-1, 0}, {1, 0}, {1, -1}, {1, 1}},           // H
    {{-1, -1}, {-1, 1}, {1, 0}, {-1, -1}},                           // D
    {{-1, -1}, {0, 1}, {1, -1}, {0.5, 0}, {-0.5, 0}},                // A
    {{0, -1}, {0, 0}, {-1, 1}, {0, 0}, {1, 1}},                      // Y
    {{0, -0.5}, {-0.25, -1}},                                        // ,
    {},                                                              // <space>
    {{0, 0}, {1, 0}, {1, -1}, {-1, -1}, {-1, 1}, {1, 1}},            // G
    {{1, 1}, {-1, 1}, {-1, 0}, {1, 0}, {-1, 0}, {-1, -1}, {1, -1}},  // E
    {{-1, -1}, {-1, 1}, {1, 1}, {1, 0}, {-1, 0}, {1, -1}},           // R
    {{0, 0}, {1, 0}, {1, -1}, {-1, -1}, {-1, 1}, {1, 1}},            // G
    {{-0.25, -0.5},
     {-0.25, 1},
     {0.25, 1},
     {0.25, -0.5},
     {-0.25, -0.5},
     {0, -0.5},
     {0, -0.75},
     {-0.25, -1},
     {0.25, -1},
     {0, -0.75}},  // !

};

std::vector<std::vector<glm::vec2>> ScaleLetters(float scale, float angle,
                                                 glm::vec2 displacement) {
  std::vector<std::vector<glm::vec2>> out_letters;
  for (auto letter : kLetters) {
    std::vector<glm::vec2> scaled_letter;
    for (auto vec : letter) {
      vec = glm::vec2(cos(angle) * vec.x - sin(angle) * vec.y,
                      sin(angle) * vec.x + cos(angle) * vec.y);
      scaled_letter.push_back(vec * scale + displacement);
    }
    out_letters.push_back(scaled_letter);
  }
  return out_letters;
}

}  // namespace

SimplePreset::SimplePreset(int width, int height) : Preset(width, height) {
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

void SimplePreset::OnUpdateGeometry() {
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

void SimplePreset::OnDrawFrame(absl::Span<const float> samples,
                               std::shared_ptr<GlobalState> state) {
  float energy = state->energy();
  float power = state->power();
  float average_power = state->average_power();
  float normalized_power =
      (average_power > 0.0f) ? power / average_power : 0.0f;

  static auto buffer_size = samples.size() / 2;
  static std::vector<glm::vec2> vertices;
  vertices.resize(buffer_size);

  static Rectangle rectangle;
  static Polyline polyline;
  static Polyline letter;

  // for (int i = 0; i < buffer_size; ++i) {
  //  float c3 = cos(energy / 10 + power / 100);
  //  float s3 = sin(energy / 10 + power / 100);
  //  float x_int = samples[i * 2] * kScaleFactor;
  //  float y_int = samples[i * 2 + 1] * kScaleFactor;

  //  float x_pos = x_int * c3 - y_int * s3;
  //  float y_pos = x_int * s3 + y_int * c3;

  //  x_pos += cos(sin(2 * energy) * 5 * energy / 1.25 + power / 100) / 2;
  //  x_pos += cos(sin(2 * energy) * 5 * energy / 5.23 + 0.5) / 5;
  //  y_pos += sin(sin(2 * energy) * 5 * energy / 1.25 + power / 100) / 2;
  //  y_pos += sin(sin(2 * energy) * 5 * energy / 5.23 + 0.5) / 5;

  //  vertices[i] = glm::vec2(x_pos, y_pos);
  //}

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

    // polyline.UpdateVertices(vertices);
    // polyline.UpdateWidth(log(normalized_power) * 50);
    // polyline.UpdateColor(HsvToRgb(glm::vec3(energy, 1, 0.5)));
    // polyline.Draw();
    //

    float a1 = M_PI * (cos(10 * energy / 0.512 + power / 10) * 0.3);
    float a2 = cos(10 * energy / 2 + power / 10) / 10;
    float a3 = sin(10 * energy / 2 + power / 10) / 10;

    a2 += cos(sin(2 * energy) * 5 * energy / 1.25 + power / 100) / 2;
    a2 += cos(sin(2 * energy) * 5 * energy / 5.23 + 0.5) / 5;
    a3 += sin(sin(2 * energy) * 5 * energy / 1.25 + power / 100) / 2;
    a3 += sin(sin(2 * energy) * 5 * energy / 5.23 + 0.5) / 5;
    auto kLetters =
        ScaleLetters(std::max<float>(0.1, log(normalized_power) * 0.3), a1,
                     glm::vec2(a2, a3));
    int letter_index = static_cast<int>(energy * 30) % kLetters.size();
    letter.UpdateVertices(kLetters[letter_index]);
    letter.UpdateWidth(std::max(0.1, log(normalized_power) * 20));
    letter.UpdateColor(HsvToRgb(glm::vec3(energy, 1, 0.5)));
    letter.Draw();

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
