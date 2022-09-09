#include "preset/fluid_preset/fluid_preset.h"

#include <algorithm>
#include <cmath>

#include "preset/fluid_preset/fluid.fsh.h"
#include "preset/fluid_preset/input.fsh.h"
#include "preset/fluid_preset/output.fsh.h"
#include "preset/fluid_preset/passthrough.vsh.h"
#include "primitive/polyline.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"
#include "util/math/math.h"
#include "util/math/vector.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;
}

FluidPreset::FluidPreset(
    std::shared_ptr<gl::GlProgram> input_program,
    std::shared_ptr<gl::GlProgram> fluid_program,
    std::shared_ptr<gl::GlProgram> output_program,
    std::shared_ptr<gl::GlRenderTarget> front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_render_target,
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      input_program_(input_program),
      fluid_program_(fluid_program),
      output_program_(output_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target) {}

absl::StatusOr<std::shared_ptr<Preset>> FluidPreset::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(
      auto input_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), input_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto fluid_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), fluid_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto output_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), output_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));

  return std::shared_ptr<FluidPreset>(new FluidPreset(
      input_program, fluid_program, output_program, front_render_target,
      back_render_target, texture_manager));
}

void FluidPreset::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void FluidPreset::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy() * 10;
  float power = state->power();

  {
    auto front_activation = front_render_target_->Activate();

    input_program_->Use();

    std::array<glm::vec2, 2> vertices = {
        UnitVectorAtAngle(energy) * 0.1f,
        UnitVectorAtAngle(energy + kPi) * 0.1f,
    };
    auto color =
        UnitVectorAtAngle(energy + kPi / 2) / 2.0f + glm::vec2(0.5, 0.5);
    Polyline polyline(glm::vec3(color, 0), vertices, 5);

    polyline.Draw();
  }

  {
    auto back_activation = back_render_target_->Activate();

    fluid_program_->Use();

    gl::GlBindUniform(fluid_program_, "input_frame_size",
                      glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(fluid_program_, "input_frame",
                                           front_render_target_,
                                           gl::GlTextureBindingOptions());

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();
  }

  {
    auto output_activation = output_render_target->Activate();
    output_program_->Use();

    gl::GlBindUniform(output_program_, "render_target_size",
                      glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(output_program_, "render_target",
                                           back_render_target_,
                                           gl::GlTextureBindingOptions());

    glViewport(0, 0, width(), height());
    rectangle_.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
  }
}

}  // namespace opendrop
