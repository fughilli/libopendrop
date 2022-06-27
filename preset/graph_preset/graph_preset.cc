#include "preset/graph_preset/graph_preset.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "preset/graph_preset/model_frag.fsh.h"
#include "preset/graph_preset/passthrough_vert.vsh.h"
#include "preset/graph_preset/tile_frag.fsh.h"
#include "preset/graph_preset/zoom_frag.fsh.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/graph/graph.h"
#include "util/graph/rendering.h"
#include "util/graph/types/color.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/samples.h"
#include "util/graph/types/texture.h"
#include "util/graph/types/types.h"
#include "util/graph/types/unitary.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"
#include "util/math/perspective.h"
#include "util/math/vector.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;

std::shared_ptr<gl::GlProgram> model;
std::shared_ptr<gl::GlProgram> zoom;
std::shared_ptr<gl::GlProgram> tile;

absl::Status InitGlPrograms() {
  if (model == nullptr) {
    ASSIGN_OR_RETURN(model,
                     gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                               model_frag_fsh::Code()));
  }
  if (zoom == nullptr) {
    ASSIGN_OR_RETURN(zoom,
                     gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                               zoom_frag_fsh::Code()));
  }
  if (tile == nullptr) {
    ASSIGN_OR_RETURN(tile,
                     gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                               tile_frag_fsh::Code()));
  }
  return absl::OkStatus();
}

}  // namespace

GraphPreset::GraphPreset(std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager) {
  graph_builder_
      .DeclareConversion<std::tuple<Unitary, Unitary>, std::tuple<Unitary>>(
          "product",
          [](std::tuple<Unitary, Unitary> in) -> std::tuple<Unitary> {
            auto& [a, b] = in;
            return std::tuple<Unitary>(a * b);
          });
  graph_builder_.DeclareProduction<std::tuple<Unitary>>(
      "random", []() -> std::tuple<Unitary> { return 5.0f; });
  graph_builder_
      .DeclareConversion<std::tuple<Monotonic, Unitary>, std::tuple<Monotonic>>(
          "scale_speed",
          [](std::tuple<Monotonic, Unitary> in) -> std::tuple<Monotonic> {
            auto& [monotonic, scale] = in;
            return std::make_tuple(Monotonic(monotonic * scale * 100));
          });
  // Configure graph.
  graph_builder_.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "fast_sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::cos(std::get<0>(in) * 30)) / 2.0f));
      });
  graph_builder_.DeclareConversion<std::tuple<Monotonic>, std::tuple<Unitary>>(
      "slow_sinusoid", [](std::tuple<Monotonic> in) -> std::tuple<Unitary> {
        return std::tuple<Unitary>(
            Unitary((1.0f + std::cos(std::get<0>(in) * 3)) / 2.0f));
      });
  graph_builder_.DeclareConversion<std::tuple<Unitary>, std::tuple<Color>>(
      "color_wheel", [](std::tuple<Unitary> in) -> std::tuple<Color> {
        glm::vec4 color =
            glm::vec4(HsvToRgb(glm::vec3(std::get<0>(in), 1.0f, 1.0f)), 1.0f);
        return std::tuple<Color>(color);
      });
  graph_builder_.DeclareConversion<
      std::tuple<Samples, Color, Monotonic, Unitary>, std::tuple<Texture>>(
      "scribble",
      [this, texture_manager](std::tuple<Samples, Color, Monotonic, Unitary> in)
          -> std::tuple<Texture> {
        const auto& [samples, color, rotation, width_scale] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = model->Activate();

        GlBindUniform(model, "model_transform",
                      RotateAround(Directions::kIntoScreen, rotation));

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        std::vector<glm::vec2> vertices(samples.samples_left.size());
        for (int i = 0; i < vertices.size(); ++i) {
          vertices[i] = {samples.samples_left[i], samples.samples_right[i]};
        }

        Polyline polyline;
        polyline.UpdateVertices(vertices);
        polyline.UpdateWidth(1 + width_scale * 30);
        polyline.UpdateColor(color.value);
        polyline.Draw();

        return std::make_tuple(tex);
      });
  graph_builder_.DeclareConversion<std::tuple<Color, Unitary, Monotonic>,
                                   std::tuple<Texture>>(
      "colored_rectangle",
      [this, texture_manager](
          std::tuple<Color, Unitary, Monotonic> in) -> std::tuple<Texture> {
        auto return_tuple = [this, texture_manager,
                             in]() -> std::tuple<Texture> {
          const auto& [color, scale, rotation] = in;
          Texture tex(width(), height(), texture_manager);

          LOG(INFO) << "inside conversion, immediately after construction: "
                       "tex has use count "
                    << tex.RenderTarget().use_count();

          auto rt_activation = tex.RenderTarget()->Activate();

          LOG(INFO) << "inside conversion, render target activated: tex "
                       "has use count "
                    << tex.RenderTarget().use_count();

          auto shader_activation = model->Activate();

          GlBindUniform(model, "model_transform",
                        ScaleTransform(scale) *
                            RotateAround(Directions::kIntoScreen, rotation));

          glClearColor(0, 0, 0, 0);
          glClear(GL_COLOR_BUFFER_BIT);

          Rectangle rectangle;
          rectangle.SetColor(color);
          rectangle.Draw();

          LOG(INFO) << "inside conversion: tex has use count "
                    << tex.RenderTarget().use_count();
          auto return_tuple = std::make_tuple(tex);
          LOG(INFO) << "inside conversion, after tuple construction: tex "
                       "has use count "
                    << tex.RenderTarget().use_count();
          return return_tuple;
        }();

        LOG(INFO)
            << "inside conversion, tex out of scope: use count in tuple is "
            << std::get<0>(return_tuple).RenderTarget().use_count();

        return return_tuple;
      });

  graph_builder_.DeclareConversion<std::tuple<Texture, Texture, Unitary>,
                                   std::tuple<Texture>>(
      "zoom",
      [this, texture_manager](
          std::tuple<Texture, Texture, Unitary> in) -> std::tuple<Texture> {
        auto& [in_tex_a, in_tex_b, rotation_coeff] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = zoom->Activate();

        GlBindUniform(zoom, "model_transform", glm::mat4(1.0f));
        GlBindRenderTargetTextureToUniform(zoom, "in_tex_a",
                                           in_tex_a.RenderTarget(),
                                           gl::GlTextureBindingOptions());
        GlBindRenderTargetTextureToUniform(zoom, "in_tex_b",
                                           in_tex_b.RenderTarget(),
                                           gl::GlTextureBindingOptions());
        GlBindUniform(zoom, "rotation_coeff", rotation_coeff.value);

        Rectangle().Draw();

        return std::make_tuple(tex);
      });

  graph_builder_.DeclareConversion<std::tuple<Texture, Unitary, Unitary>,
                                   std::tuple<Texture>>(
      "tile",
      [this, texture_manager](
          std::tuple<Texture, Unitary, Unitary> in) -> std::tuple<Texture> {
        auto& [in_tex, sample_scale_x, sample_scale_y] = in;
        glm::vec2 sample_scale = {sample_scale_x, sample_scale_y};
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = tile->Activate();

        GlBindUniform(tile, "model_transform", glm::mat4(1.0f));
        GlBindRenderTargetTextureToUniform(
            tile, "in_tex", in_tex.RenderTarget(),
            gl::GlTextureBindingOptions{
                .sampling_mode = gl::GlTextureSamplingMode::kMirrorWrap});
        GL_BIND_LOCAL(tile, sample_scale);

        Rectangle().Draw();

        return std::make_tuple(tex);
      });

  evaluation_graph_ =
      graph_builder_
          .Bridge(
              ConstructTypes<Samples, Monotonic, Unitary, Unitary, Unitary>(),
              ConstructTypes<Texture>())
          .value();

  ax::NodeEditor::Config config{};
  config.SettingsFile = "graph_nodes.json";
  editor_context_ = ax::NodeEditor::CreateEditor(&config);
}

GraphPreset::~GraphPreset() {
  ax::NodeEditor::DestroyEditor(editor_context_);
  LOG(INFO) << "Disposing GraphPreset";
}

absl::StatusOr<std::shared_ptr<Preset>> GraphPreset::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  RETURN_IF_ERROR(InitGlPrograms());
  return std::shared_ptr<GraphPreset>(new GraphPreset(texture_manager));
}

void GraphPreset::OnUpdateGeometry() { glViewport(0, 0, width(), height()); }

void GraphPreset::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  ImGui::Begin("Graph Viewer", nullptr, ImGuiWindowFlags_NoScrollbar);
  if (ImGui::Button("Clear")) {
    evaluation_graph_ = Graph(nullptr);
    graph_builder_.MaybeGc();
  }
  if (ImGui::Button("Again"))
    evaluation_graph_ =
        graph_builder_
            .Bridge(
                ConstructTypes<Samples, Monotonic, Unitary, Unitary, Unitary>(),
                ConstructTypes<Texture>())
            .value();
  ImGui::Checkbox("Evaluate?", &evaluate_);
  RenderGraph(editor_context_, evaluation_graph_);
  ImGui::End();

  texture_manager()->PrintState();
  graph_builder_.PrintState();

  if (evaluate_) {
    evaluation_graph_.Evaluate(
        std::tuple<Samples, Monotonic, Unitary, Unitary, Unitary>(
            Samples{.samples_left = state->left_channel(),
                    .samples_right = state->right_channel()},
            state->energy(), state->bass(), state->mid(), state->treble()));
    Texture tex = std::get<0>(evaluation_graph_.Result<Texture>());

    {
      auto output_activation = output_render_target->Activate();
      Blit(tex);
    }

    // LOG(INFO) << "Exiting scope of `tex`";
  }
}

}  // namespace opendrop
