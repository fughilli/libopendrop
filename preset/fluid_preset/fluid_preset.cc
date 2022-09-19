#include "preset/fluid_preset/fluid_preset.h"

#include <algorithm>
#include <cmath>

#include "debug/control_injector.h"
#include "preset/fluid_preset/advection.fsh.h"
#include "preset/fluid_preset/blit.fsh.h"
#include "preset/fluid_preset/clear.fsh.h"
#include "preset/fluid_preset/common_vertex.vsh.h"
#include "preset/fluid_preset/curl.fsh.h"
#include "preset/fluid_preset/divergence.fsh.h"
#include "preset/fluid_preset/gradient_subtract.fsh.h"
#include "preset/fluid_preset/input.fsh.h"
#include "preset/fluid_preset/pressure.fsh.h"
#include "preset/fluid_preset/vorticity.fsh.h"
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

absl::StatusOr<std::shared_ptr<Preset>> FluidPreset::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  return std::shared_ptr<FluidPreset>(new FluidPreset(texture_manager));
}

FluidPreset::FluidPreset(std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager) {
  render_targets_[0] =
      gl::GlRenderTarget::MakeShared(0, 0, texture_manager).value();
  for (int i = 1; i < render_targets_.size(); ++i) {
    gl::GlRenderTarget::Options options;
    options.type = gl::GlRenderTarget::TextureType::kHalfFloat;
    render_targets_[i] =
        gl::GlRenderTarget::MakeShared(0, 0, texture_manager, options).value();
  }

  int i = 0;
  for (auto& code :
       {curl_fsh::Code(), vorticity_fsh::Code(), divergence_fsh::Code(),
        clear_fsh::Code(), pressure_fsh::Code(), gradient_subtract_fsh::Code(),
        advection_fsh::Code(), blit_fsh::Code(), input_fsh::Code()}) {
    programs_[i++] =
        gl::GlProgram::MakeShared(common_vertex_vsh::Code(), code).value();
  }
}

void FluidPreset::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  for (auto render_target : render_targets_) {
    if (render_target != nullptr) {
      render_target->UpdateGeometry(width(), height());
    }
  }
}

void FluidPreset::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  auto& [dye_target, curl_target, velocity_target, divergence_target,
         pressure_target] = render_targets_;
  auto& [curl_program, vorticity_program, divergence_program, clear_program,
         pressure_program, gradient_subtract_program, advection_program,
         blit_program, input_program] = programs_;

  if (SIGINJECT_TRIGGER("insert_velocity")) {
    auto velocity_activation = velocity_target->Activate();
    input_program->Use();

    gl::GlBindUniform(input_program, "size", glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(input_program, "last_frame",
                                           velocity_target, {.back = true});
    gl::GlBindUniform(input_program, "splat_center",
                      glm::vec2(SIGINJECT_OVERRIDE("posx", 0.0f, -1.0f, 1.0f),
                                SIGINJECT_OVERRIDE("posy", 0.0f, -1.0f, 1.0f)));
    gl::GlBindUniform(input_program, "splat_velocity",
                      UnitVectorAtAngle(state->energy() * 10) * 1000.0f);

    Rectangle().Draw();

    // float angle = state->energy() * 10;

    // std::array<glm::vec2, 2> points = {UnitVectorAtAngle(angle) * 0.2f,
    //                                   UnitVectorAtAngle(angle + kPi) * 0.2f};
    // glm::vec2 color = UnitVectorAtAngle(angle + kPi / 2.0f) * 10.0f;
    // Polyline polyline(glm::vec3(color, 0.0f), points, /*width=*/4);
    // polyline.Draw();

    velocity_target->swap();
  }  // Looks good.

  // if (SIGINJECT_TRIGGER("clear_all")) {
  //  {
  //    auto pressure_activation = pressure_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    pressure_target->swap();
  //  }
  //  {
  //    auto velocity_activation = velocity_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    velocity_target->swap();
  //  }
  //  {
  //    auto curl_activation = curl_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    curl_target->swap();
  //  }
  //  {
  //    auto divergence_activation = divergence_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    divergence_target->swap();
  //  }
  //  {
  //    auto pressure_activation = pressure_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    pressure_target->swap();
  //  }
  //  {
  //    auto velocity_activation = velocity_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    velocity_target->swap();
  //  }
  //  {
  //    auto curl_activation = curl_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    curl_target->swap();
  //  }
  //  {
  //    auto divergence_activation = divergence_target->Activate();
  //    clear_program->Use();
  //    Rectangle().Draw();
  //    divergence_target->swap();
  //  }
  //}

  {
    auto curl_activation = curl_target->Activate();
    curl_program->Use();
    gl::GlBindUniform(curl_program, "size", glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(curl_program, "velocity",
                                           velocity_target, {.back = true});
    Rectangle().Draw();
    curl_target->swap();
  }

  {
    auto velocity_activation = velocity_target->Activate();
    vorticity_program->Use();
    gl::GlBindUniform(vorticity_program, "size", glm::ivec2(width(), height()));
    gl::GlBindUniform(vorticity_program, "curl_coeff", 1.0f);
    gl::GlBindUniform(vorticity_program, "dt", state->dt() * 10);
    gl::GlBindRenderTargetTextureToUniform(vorticity_program, "velocity",
                                           velocity_target, {.back = true});
    gl::GlBindRenderTargetTextureToUniform(vorticity_program, "curl",
                                           curl_target, {.back = true});
    Rectangle().Draw();
    velocity_target->swap();
  }

  {
    auto divergence_activation = divergence_target->Activate();
    divergence_program->Use();
    gl::GlBindUniform(divergence_program, "size",
                      glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(divergence_program, "velocity",
                                           velocity_target, {.back = true});
    Rectangle().Draw();
    divergence_target->swap();
  }

  {
    auto pressure_activation = pressure_target->Activate();
    clear_program->Use();
    Rectangle().Draw();
    pressure_target->swap();
  }

  for (int i = 0; i < 20; ++i) {
    auto pressure_activation = pressure_target->Activate();
    pressure_program->Use();
    gl::GlBindUniform(pressure_program, "size", glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(pressure_program, "divergence",
                                           divergence_target, {.back = true});
    gl::GlBindRenderTargetTextureToUniform(pressure_program, "pressure",
                                           pressure_target, {.back = true});
    Rectangle().Draw();
    pressure_target->swap();
  }

  {
    auto velocity_activation = velocity_target->Activate();
    gradient_subtract_program->Use();
    gl::GlBindUniform(gradient_subtract_program, "size",
                      glm::ivec2(width(), height()));
    gl::GlBindRenderTargetTextureToUniform(
        gradient_subtract_program, "pressure", pressure_target, {.back = true});
    gl::GlBindRenderTargetTextureToUniform(
        gradient_subtract_program, "velocity", velocity_target, {.back = true});
    Rectangle().Draw();
    velocity_target->swap();
  }

  {
    auto velocity_activation = velocity_target->Activate();
    advection_program->Use();
    gl::GlBindUniform(advection_program, "size", glm::ivec2(width(), height()));
    gl::GlBindUniform(advection_program, "dissipation", 0.1f);
    gl::GlBindUniform(advection_program, "dt", state->dt() * 10);
    gl::GlBindRenderTargetTextureToUniform(advection_program, "velocity",
                                           velocity_target, {.back = true});
    gl::GlBindRenderTargetTextureToUniform(
        advection_program, "source", velocity_target,
        {.sampling_mode = gl::GlTextureSamplingMode::kWrap, .back = true});
    Rectangle().Draw();
    velocity_target->swap();
  }

  {
    auto output_activation = output_render_target->Activate();

    blit_program->Use();

    gl::GlBindUniform(blit_program, "size", glm::ivec2(width(), height()));
    // gl::GlBindRenderTargetTextureToUniform(blit_program, "source_texture",
    //                                       dye_target, {.back = true});
    gl::GlBindRenderTargetTextureToUniform(blit_program, "source_texture",
                                           velocity_target, {.back = true});

    Rectangle().Draw();
  }

  //{
  //  auto dye_activation = dye_target->Activate();
  //  advection_program->Use();
  //  gl::GlBindUniform(advection_program, "size", glm::ivec2(width(),
  //  height())); gl::GlBindUniform(advection_program, "dissipation", 0.0f);
  //  gl::GlBindUniform(advection_program, "dt", state->dt());
  //  gl::GlBindRenderTargetTextureToUniform(advection_program, "velocity",
  //                                         velocity_target, {.back = true});
  //  gl::GlBindRenderTargetTextureToUniform(advection_program, "source",
  //                                         dye_target, {.back = true});
  //  Rectangle().Draw();
  //  dye_target->swap();
  //}
}

}  // namespace opendrop
