#include "libopendrop/preset/alien_rorschach/alien_rorschach.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <vector>

#include "libopendrop/preset/alien_rorschach/composite.fsh.h"
#include "libopendrop/preset/alien_rorschach/passthrough.vsh.h"
#include "libopendrop/preset/alien_rorschach/warp.fsh.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.20f;
}

AlienRorschach::AlienRorschach(
    std::shared_ptr<gl::GlTextureManager> texture_manager, int width,
    int height)
    : Preset(texture_manager, width, height) {
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

  front_render_target_ = std::make_shared<gl::GlRenderTarget>(
      width, height, this->texture_manager());
  back_render_target_ = std::make_shared<gl::GlRenderTarget>(
      width, height, this->texture_manager());
}

void AlienRorschach::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void AlienRorschach::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    std::shared_ptr<gl::GlRenderTarget> output_render_target) {
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

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    int texture_size_location = glGetUniformLocation(
        warp_program_->program_handle(), "last_frame_size");
    int power_location =
        glGetUniformLocation(warp_program_->program_handle(), "power");
    int energy_location =
        glGetUniformLocation(warp_program_->program_handle(), "energy");
    glUniform1f(power_location, power);
    glUniform1f(energy_location, energy);
    glUniform2i(texture_size_location, width(), height());
    GlBindRenderTargetTextureToUniform(warp_program_, "last_frame",
                                       front_render_target_);

    rectangle.Draw();

    for (int j = 0; j < 4; j++) {
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

        x_pos += cos(energy * 10 + (j / 4.0 * M_PI * 2)) / 2;
        y_pos += sin(energy * 10 + (j / 4.0 * M_PI * 2)) / 2;

        vertices[i] = glm::vec2(x_pos, y_pos);
      }

      polyline.UpdateVertices(vertices);
      polyline.UpdateWidth(normalized_power * 5);
      polyline.UpdateColor(
          HsvToRgb(glm::vec3(energy * 10 + (j / 4.0f), 1, 0.5)));
      polyline.Draw();
    }

    glFlush();
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_);
    int texture_size_location = glGetUniformLocation(
        composite_program_->program_handle(), "render_target_size");
    glUniform2i(texture_size_location, width(), height());
    int power_location =
        glGetUniformLocation(composite_program_->program_handle(), "power");
    int energy_location =
        glGetUniformLocation(composite_program_->program_handle(), "energy");
    glUniform1f(power_location, power);
    glUniform1f(energy_location, energy);

    glViewport(0, 0, width(), height());
    rectangle.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
    glFlush();
  }
}

}  // namespace opendrop