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
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"
#include "libopendrop/util/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.20f;
}

Kaleidoscope::Kaleidoscope(
    std::shared_ptr<gl::GlProgram> warp_program,
    std::shared_ptr<gl::GlProgram> composite_program,
    std::shared_ptr<gl::GlRenderTarget> front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_render_target,
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target) {}

absl::StatusOr<std::shared_ptr<Preset>> Kaleidoscope::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  std::shared_ptr<gl::GlProgram> warp_program, composite_program;
  ASSIGN_OR_RETURN(
      warp_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));

  std::shared_ptr<gl::GlRenderTarget> front_render_target, back_render_target;
  ASSIGN_OR_RETURN(front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));

  return std::shared_ptr<Kaleidoscope>(
      new Kaleidoscope(warp_program, composite_program, front_render_target,
                       back_render_target, texture_manager));
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

void Kaleidoscope::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
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

    LOG(DEBUG) << "Using program";
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
      polyline.UpdateColor(HsvToRgb(glm::vec3(energy, 1, 0.5)));
      polyline.Draw();
    }

    glFlush();
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();
    LOG(DEBUG) << "Using program";
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_);
    int texture_size_location = glGetUniformLocation(
        composite_program_->program_handle(), "render_target_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    glUniform2i(texture_size_location, width(), height());
    int power_location =
        glGetUniformLocation(composite_program_->program_handle(), "power");
    int energy_location =
        glGetUniformLocation(composite_program_->program_handle(), "energy");
    LOG(DEBUG) << "Got locations for power: " << power_location
               << " and energy: " << energy_location;
    glUniform1f(power_location, power);
    glUniform1f(energy_location, energy);
    LOG(DEBUG) << "Power: " << power << " energy: " << energy;
    glUniform1f(
        glGetUniformLocation(composite_program_->program_handle(), "alpha"),
        alpha);

    glViewport(0, 0, width(), height());
    rectangle.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
    glFlush();
  }
}

}  // namespace opendrop
