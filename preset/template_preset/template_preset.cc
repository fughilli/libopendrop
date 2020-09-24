#include "libopendrop/preset/template_preset/template_preset.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <vector>

#include "libopendrop/preset/template_preset/composite.fsh.h"
#include "libopendrop/preset/template_preset/passthrough.vsh.h"
#include "libopendrop/preset/template_preset/warp.fsh.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;
}

TemplatePreset::TemplatePreset(int width, int height) : Preset(width, height) {
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

void TemplatePreset::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void TemplatePreset::OnDrawFrame(absl::Span<const float> samples,
                               std::shared_ptr<GlobalState> state) {
  float energy = state->energy();
  float power = state->power();

  static auto buffer_size = samples.size() / 2;
  static std::vector<glm::vec2> vertices;
  vertices.resize(buffer_size);

  static Rectangle rectangle;
  static Polyline polyline;

  float cos_energy = cos(energy * 10 + power * 10);
  float sin_energy = sin(energy * 10 + power * 10);
  float total_scale_factor = kScaleFactor * (0.7 + 0.3 * cos_energy);

  for (int i = 0; i < buffer_size; ++i) {
    float x_scaled = samples[i * 2] * total_scale_factor;
    float y_scaled = samples[i * 2 + 1] * total_scale_factor;

    float x_pos = x_scaled * cos_energy - y_scaled * sin_energy;
    float y_pos = x_scaled * sin_energy + y_scaled * cos_energy;

    vertices[i] = glm::vec2(x_pos, y_pos);
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    glUniform1f(glGetUniformLocation(warp_program_->program_handle(), "power"),
                power);
    glUniform1f(glGetUniformLocation(warp_program_->program_handle(), "energy"),
                energy);
    glUniform2i(glGetUniformLocation(warp_program_->program_handle(),
                                     "last_frame_size"),
                width(), height());
    int texture_number = 0;
    glActiveTexture(GL_TEXTURE0 + texture_number);
    glBindTexture(GL_TEXTURE_2D, front_render_target_->texture_handle());
    glUniform1i(
        glGetUniformLocation(warp_program_->program_handle(), "last_frame"),
        texture_number);

    // Force all fragments to draw with a full-screen rectangle.
    rectangle.Draw();

    // Draw the waveform.
    polyline.UpdateVertices(vertices);
    polyline.UpdateWidth(2 + power * 10);
    polyline.UpdateColor(HsvToRgb(glm::vec3(energy, 1, 0.5)));
    polyline.Draw();

    glFlush();
  }

  composite_program_->Use();
  int texture_number = 0;
  glActiveTexture(GL_TEXTURE0 + texture_number);
  glBindTexture(GL_TEXTURE_2D, back_render_target_->texture_handle());
  glUniform1i(glGetUniformLocation(composite_program_->program_handle(),
                                   "render_target"),
              texture_number);
  glUniform2i(glGetUniformLocation(composite_program_->program_handle(),
                                   "render_target_size"),
              width(), height());

  glViewport(0, 0, width(), height());
  rectangle.Draw();

  back_render_target_->swap_texture_unit(front_render_target_.get());
  glFlush();
}

}  // namespace opendrop
