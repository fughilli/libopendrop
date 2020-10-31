#include "libopendrop/preset/rotary_transporter/rotary_transporter.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>

#include "libopendrop/preset/rotary_transporter/composite.fsh.h"
#include "libopendrop/preset/rotary_transporter/passthrough.vsh.h"
#include "libopendrop/preset/rotary_transporter/warp.fsh.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"
#include "libopendrop/util/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;
}

RotaryTransporter::RotaryTransporter(
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

absl::StatusOr<std::shared_ptr<Preset>> RotaryTransporter::MakeShared(
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

  return std::shared_ptr<RotaryTransporter>(new RotaryTransporter(
      warp_program, composite_program, front_render_target, back_render_target,
      texture_manager));
}

void RotaryTransporter::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void RotaryTransporter::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy();
  float power = state->power();

  auto buffer_size = samples.size() / 2;
  vertices_.resize(buffer_size);

  float cos_energy = cos(energy * 10 + power * 10);
  float sin_energy = sin(energy * 10 + power * 10);
  float total_scale_factor = kScaleFactor * (0.7 + 0.3 * cos_energy);

  for (int i = 0; i < buffer_size; ++i) {
    float x_scaled = samples[i * 2] * total_scale_factor;
    float y_scaled = samples[i * 2 + 1] * total_scale_factor;

    float x_pos = x_scaled * cos_energy - y_scaled * sin_energy;
    float y_pos = x_scaled * sin_energy + y_scaled * cos_energy;

    vertices_[i] = glm::vec2(x_pos, y_pos);
  }

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
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();

    // Draw the waveform.
    constexpr int kSymmetry = 3;
    for (int i = 0; i < kSymmetry; ++i) {
      std::vector<glm::vec2> vertices_rotary(vertices_.size(), glm::vec2(0, 0));
      for (int j = 0; j < vertices_rotary.size(); ++j) {
        vertices_rotary[j] =
            vertices_[j] +
            glm::vec2(cos(i * (M_PI * 2 / kSymmetry) + energy * 40),
                      sin(i * (M_PI * 2 / kSymmetry) + energy * 40)) *
                0.3f;
      }
      polyline_.UpdateVertices(vertices_rotary);
      polyline_.UpdateWidth(2 + power * 10);
      polyline_.UpdateColor(HsvToRgb(glm::vec3(energy + 0.0888 * i, 1, 0.5)));
      polyline_.Draw();
    }

    glFlush();
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();
    LOG(DEBUG) << "Using program";
    int texture_size_location = glGetUniformLocation(
        composite_program_->program_handle(), "render_target_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    glUniform2i(texture_size_location, width(), height());
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());
    glUniform1f(
        glGetUniformLocation(composite_program_->program_handle(), "alpha"),
        alpha);

    glViewport(0, 0, width(), height());
    rectangle_.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
    glFlush();
  }
}

}  // namespace opendrop
