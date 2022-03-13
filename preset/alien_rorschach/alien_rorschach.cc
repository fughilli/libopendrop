#include "preset/alien_rorschach/alien_rorschach.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>

#include "preset/alien_rorschach/composite.fsh.h"
#include "preset/alien_rorschach/passthrough.vsh.h"
#include "preset/alien_rorschach/warp.fsh.h"
#include "util/colors.h"
#include "util/gl_util.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.20f;
}

AlienRorschach::AlienRorschach(
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

absl::StatusOr<std::shared_ptr<Preset>> AlienRorschach::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(
      auto warp_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));

  return std::shared_ptr<AlienRorschach>(
      new AlienRorschach(warp_program, composite_program, front_render_target,
                         back_render_target, texture_manager));
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
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy() / 10;
  float power = state->power();
  float average_power = state->average_power();
  float normalized_power = SafeDivide(power, average_power);

  auto buffer_size = samples.size() / 2;
  vertices_.resize(buffer_size);

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    GlBindUniform(warp_program_, "power", power);
    GlBindUniform(warp_program_, "energy", energy);
    GlBindUniform(warp_program_, "last_frame_size",
                  glm::ivec2(width(), height()));
    GlBindRenderTargetTextureToUniform(warp_program_, "last_frame",
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());

    rectangle_.Draw();

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

        vertices_[i] = glm::vec2(x_pos, y_pos);
      }

      polyline_.UpdateVertices(vertices_);
      polyline_.UpdateWidth(normalized_power * 5);
      polyline_.UpdateColor(
          HsvToRgb(glm::vec3(energy * 1 + (j / 4.0f), 1, 0.5)));
      polyline_.Draw();
    }
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();

    GlBindUniform(composite_program_, "power", power);
    GlBindUniform(composite_program_, "energy", energy);
    GlBindUniform(composite_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(composite_program_, "alpha", alpha);
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());

    glViewport(0, 0, width(), height());
    rectangle_.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
  }
}

}  // namespace opendrop
