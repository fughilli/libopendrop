#include "preset/graph_preset/graph_preset.h"

#include <algorithm>
#include <cmath>

#include "preset/graph_preset/composite.fsh.h"
#include "preset/graph_preset/passthrough.vsh.h"
#include "preset/graph_preset/warp.fsh.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/graph/graph.h"
#include "util/graph/types/color.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/texture.h"
#include "util/graph/types/types.h"
#include "util/graph/types/unitary.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;
}

GraphPreset::GraphPreset(
    std::shared_ptr<gl::GlProgram> warp_program,
    std::shared_ptr<gl::GlProgram> composite_program,
    std::shared_ptr<gl::GlRenderTarget> front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_render_target,
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target) {
  graph_builder_.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::cos(std::get<0>(in))) / 2.0f));
      });
  graph_builder_.DeclareConversion<std::tuple<Unitary>, std::tuple<Color>>(
      "color_wheel", [](std::tuple<Unitary> in) -> std::tuple<Color> {
        glm::vec4 color =
            glm::vec4(HsvToRgb(glm::vec3(std::get<0>(in), 1.0f, 1.0f)), 1.0f);
        return std::tuple<Color>(color);
      });
  graph_builder_.DeclareConversion<std::tuple<Color>, std::tuple<Texture>>(
      "single_color_texture",
      [this, texture_manager](std::tuple<Color> in) -> std::tuple<Texture> {
        Texture tex(width(), height(), texture_manager);

        auto activation = tex.RenderTarget()->Activate();

        glm::vec4 color = std::get<0>(in);

        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        return tex;
      });

  evaluation_graph_ = graph_builder_.Bridge(ConstructTypes<Monotonic>(),
                                            ConstructTypes<Texture>());
}

absl::StatusOr<std::shared_ptr<Preset>> GraphPreset::MakeShared(
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

  return std::shared_ptr<GraphPreset>(
      new GraphPreset(warp_program, composite_program, front_render_target,
                      back_render_target, texture_manager));
}

void GraphPreset::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void GraphPreset::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  evaluation_graph_.Evaluate(std::tuple<Monotonic>(state->energy()));
  Texture tex = std::get<0>(evaluation_graph_.Result<Texture>());

  {
    auto output_activation = output_render_target->Activate();
    Blit(tex);
  }
}

}  // namespace opendrop
