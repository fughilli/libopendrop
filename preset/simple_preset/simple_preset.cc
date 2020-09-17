#include "libopendrop/preset/simple_preset/simple_preset.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

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
}

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

void SimplePreset::OnDrawFrame(absl::Span<const float> samples,
                               std::shared_ptr<GlobalState> state) {
  float energy = state->energy();
  float power = state->power();

  auto buffer_size = samples.size() / 2;
  std::vector<glm::vec2> vertices;
  vertices.resize(buffer_size);

  for (int i = 0; i < buffer_size; ++i) {
    float c3 = cos(energy / 10 + power / 100);
    float s3 = sin(energy / 10 + power / 100);
    float x_int = samples[i * 2] * kScaleFactor;
    float y_int = samples[i * 2 + 1] * kScaleFactor;

    float x_pos = x_int * c3 - y_int * s3;
    float y_pos = x_int * s3 + y_int * c3;

    x_pos += cos(energy / 1.25 + power / 100) / 2;
    x_pos += cos(energy / 5.23 + 0.5) / 5;
    y_pos += sin(energy / 1.25 + power / 100) / 2;
    y_pos += sin(energy / 5.23 + 0.5) / 5;

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

    Rectangle().Draw();
    Polyline(vertices, energy * 30).Draw();

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
  Rectangle().Draw();

  back_render_target_->swap_texture_unit(front_render_target_.get());
  glFlush();
}

}  // namespace opendrop
