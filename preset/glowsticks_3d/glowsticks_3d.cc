#include "libopendrop/preset/glowsticks_3d/glowsticks_3d.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <array>
#include <vector>

#include "libopendrop/preset/glowsticks_3d/composite.fsh.h"
#include "libopendrop/preset/glowsticks_3d/passthrough.vsh.h"
#include "libopendrop/preset/glowsticks_3d/warp.fsh.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/primitive/ribbon.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

namespace {
// Whether or not to draw the segments of the rotating armatures that describe
// the motion of the ribbon.
constexpr bool kDrawDebugSegments = false;
}  // namespace

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
  float average_power = state->average_power();
  float energy = state->energy();
  float normalized_energy = state->normalized_energy();
  float power = state->power();

  static auto buffer_size = samples.size() / 2;
  static std::vector<glm::vec2> vertices;
  vertices.resize(buffer_size);

  static Rectangle rectangle;
  static Ribbon ribbon(glm::vec3(), 50);
  static Polyline debug_segments;

  // TODO: Normalize by calculating the approximate arclength of the new
  // segment, and interpolate the argument to the sinusoids (prevent crunchy
  // ribbons).
  static float oscillatory_energy = 0.0f;
  static float oscillatory_energy_2 = 0.0f;
  oscillatory_energy += 21.42 * (power / average_power) * sin(energy * 8) / 20;
  oscillatory_energy_2 +=
      7.369 * (power / average_power) * sin(energy * 19) / 20;

  constexpr float kDivisor = 12.0f;
  constexpr float kDivisor2 = 1.0f;
  constexpr float kScaleFactor = 0.6f;
  constexpr float kRibbonSegmentOffset = 0.2f * kScaleFactor;

  auto first_segment =
      UnitVectorAtAngle(oscillatory_energy / kDivisor) * 0.4f * kScaleFactor;
  auto second_segment =
      UnitVectorAtAngle(oscillatory_energy_2 / kDivisor) * 0.3f * kScaleFactor;
  auto third_segment =
      UnitVectorAtAngle(31.115 * normalized_energy / 100 / kDivisor);

  float ribbon_width = 0.1 + 0.03f * sin(energy / kDivisor2) * kScaleFactor;

  std::array<glm::vec2, 4> debug_segment_points = {
      glm::vec2(0, 0), first_segment, first_segment + second_segment,
      first_segment + second_segment + third_segment * kRibbonSegmentOffset};

  ribbon.AppendSegment(
      {first_segment + second_segment + third_segment * kRibbonSegmentOffset,
       first_segment + second_segment +
           third_segment * (kRibbonSegmentOffset + ribbon_width)});

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
    ribbon.UpdateColor(HsvToRgb(glm::vec3(energy * 10, 1, 0.5)));
    ribbon.Draw();

    if (kDrawDebugSegments) {
      debug_segments.UpdateVertices(debug_segment_points);
      debug_segments.UpdateColor(glm::vec3(1, 1, 1));
      debug_segments.UpdateWidth(1);
      debug_segments.Draw();
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
