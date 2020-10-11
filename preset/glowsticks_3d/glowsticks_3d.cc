#include "libopendrop/preset/glowsticks_3d/glowsticks_3d.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <vector>

#include "libopendrop/preset/glowsticks_3d/composite.fsh.h"
#include "libopendrop/preset/glowsticks_3d/passthrough.vsh.h"
#include "libopendrop/preset/glowsticks_3d/warp.fsh.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/primitive/ribbon.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

Glowsticks3d::Glowsticks3d(
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager) {
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
      width(), height(), this->texture_manager());
  back_render_target_ = std::make_shared<gl::GlRenderTarget>(
      width(), height(), this->texture_manager());
}

void Glowsticks3d::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

glm::vec2 Rotate2d(glm::vec2 vector, float angle) {
  float cos_angle = cos(angle);
  float sin_angle = cos(angle);

  return glm::vec2(vector.x * cos_angle - vector.y * sin_angle,
                   vector.x * sin_angle + vector.y * cos_angle);
}

glm::vec2 UnitVectorAtAngle(float angle) {
  return glm::vec2(cos(angle), sin(angle));
}

void Glowsticks3d::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy();
  float power = state->power();

  static auto buffer_size = samples.size() / 2;
  static std::vector<glm::vec2> vertices;
  vertices.resize(buffer_size);

  static Rectangle rectangle;
  static Ribbon ribbon(glm::vec3(), 40);

  static float oscillatory_energy = 0.0f;
  static float oscillatory_energy_2 = 0.0f;
  oscillatory_energy += 21.42 * power * sin(energy / 45.12);
  oscillatory_energy_2 += 7.369 * power * sin(energy / 35.124);

  constexpr float kDivisor = 3.0f;

  auto first_segment = UnitVectorAtAngle(oscillatory_energy / kDivisor) * 0.2f;
  auto second_segment =
      UnitVectorAtAngle(oscillatory_energy_2 / kDivisor) * 0.2f;
  auto third_segment = UnitVectorAtAngle(31.115 * energy / kDivisor) * 0.3f;

  ribbon.AppendSegment(
      {first_segment + second_segment + third_segment,
       first_segment + second_segment +
           (third_segment * (0.7f + 1.0f * sin(energy * kDivisor)))});

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

    // Force all fragments to draw with a full-screen rectangle.
    rectangle.Draw();

    // Draw the waveform.
    ribbon.UpdateColor(HsvToRgb(glm::vec3(energy, 1, 0.5)));
    ribbon.Draw();

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
                                       back_render_target_);
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
