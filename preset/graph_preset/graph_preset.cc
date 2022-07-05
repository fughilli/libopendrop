#include "preset/graph_preset/graph_preset.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "preset/graph_preset/displace_frag.fsh.h"
#include "preset/graph_preset/kaleidoscope_frag.fsh.h"
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
#include "util/math/coefficients.h"
#include "util/math/math.h"
#include "util/math/perspective.h"
#include "util/math/vector.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;

std::shared_ptr<gl::GlProgram> model;
std::shared_ptr<gl::GlProgram> zoom;
std::shared_ptr<gl::GlProgram> tile;
std::shared_ptr<gl::GlProgram> kaleidoscope;
std::shared_ptr<gl::GlProgram> displace;

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
  if (kaleidoscope == nullptr) {
    ASSIGN_OR_RETURN(kaleidoscope,
                     gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                               kaleidoscope_frag_fsh::Code()));
  }
  if (displace == nullptr) {
    ASSIGN_OR_RETURN(displace,
                     gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                               displace_frag_fsh::Code()));
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
      "random", []() -> std::tuple<Unitary> {
        return Coefficients::Random<1>(0.0f, 1.0f)[0];
      });
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
  graph_builder_.DeclareConversion<std::tuple<Monotonic>, std::tuple<Color>>(
      "color_wheel", [](std::tuple<Monotonic> in) -> std::tuple<Color> {
        glm::vec4 color =
            glm::vec4(HsvToRgb(glm::vec3(std::get<0>(in), 1.0f, 1.0f)), 1.0f);
        return std::tuple<Color>(color);
      });
  graph_builder_.DeclareConversion<
      std::tuple<Samples, Color, Monotonic, Unitary, Unitary>,
      std::tuple<Texture>>(
      "sample_ring",
      [this, texture_manager](
          std::tuple<Samples, Color, Monotonic, Unitary, Unitary> in)
          -> std::tuple<Texture> {
        const auto& [samples, color, rotation, radius_scale, width_scale] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = model->Activate();

        GlBindUniform(model, "model_transform",
                      RotateAround(Directions::kIntoScreen, rotation));

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        std::vector<glm::vec2> vertices(samples.samples_left.size());
        for (int i = 0; i < vertices.size(); ++i) {
          float theta = kPi * 2 * i / vertices.size();
          vertices[i] = UnitVectorAtAngle(theta);
          vertices[i] =
              vertices[i] * (radius_scale + 0.1f * samples.samples_left[i]);
          // {samples.samples_left[i], samples.samples_right[i]};
        }

        Polyline polyline;
        polyline.UpdateVertices(vertices);
        polyline.UpdateWidth(2 + width_scale * 30);
        polyline.UpdateColor(color.value);
        polyline.Draw();

        return std::make_tuple(tex);
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
        polyline.UpdateWidth(2 + width_scale * 30);
        polyline.UpdateColor(color.value);
        polyline.Draw();

        return std::make_tuple(tex);
      });
  graph_builder_.DeclareConversion<std::tuple<Color, Unitary, Monotonic>,
                                   std::tuple<Texture>>(
      "colored_rectangle",
      [this, texture_manager](
          std::tuple<Color, Unitary, Monotonic> in) -> std::tuple<Texture> {
        const auto& [color, scale, rotation] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();

        auto shader_activation = model->Activate();

        GlBindUniform(model, "model_transform",
                      ScaleTransform(scale) *
                          RotateAround(Directions::kIntoScreen, rotation));

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        Rectangle rectangle;
        rectangle.SetColor(color);
        rectangle.Draw();

        auto return_tuple = std::make_tuple(tex);
        return return_tuple;
      });

  graph_builder_.DeclareConversion<
      std::tuple<Texture, Texture, Unitary, Unitary, Unitary, Unitary>,
      std::tuple<Texture>>(
      "zoom",
      [this, texture_manager](
          std::tuple<Texture, Texture, Unitary, Unitary, Unitary, Unitary> in)
          -> std::tuple<Texture> {
        auto& [in_tex_a, in_tex_b, rotation_coeff, zoom_coeff, zoom_center_x,
               zoom_center_y] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = zoom->Activate();

        GlBindUniform(zoom, "model_transform", glm::mat4(1.0f));
        GlBindRenderTargetTextureToUniform(
            zoom, "in_tex_a", in_tex_a.RenderTarget(),
            gl::GlTextureBindingOptions{
                .sampling_mode = gl::GlTextureSamplingMode::kClampToBorder,
            });
        GlBindRenderTargetTextureToUniform(
            zoom, "in_tex_b", in_tex_b.RenderTarget(),
            gl::GlTextureBindingOptions{
                .sampling_mode = gl::GlTextureSamplingMode::kClampToBorder,
            });
        glm::vec2 zoom_center =
            glm::vec2(zoom_center_x - 0.5f, zoom_center_y - 0.5f) * 2.0f;
        GlBindUniform(zoom, "rotation_coeff",
                      (0.5f - rotation_coeff.value) / 2);
        GlBindUniform(zoom, "zoom_coeff", Lerp(0.9f, 1.1f, zoom_coeff.value));
        GlBindUniform(zoom, "zoom_center", zoom_center);

        Rectangle().Draw();

        return std::make_tuple(tex);
      });

  graph_builder_.DeclareConversion<std::tuple<Texture, Unitary, Unitary>,
                                   std::tuple<Texture>>(
      "tile",
      [this, texture_manager](
          std::tuple<Texture, Unitary, Unitary> in) -> std::tuple<Texture> {
        auto& [in_tex, sample_scale_x, sample_scale_y] = in;
        glm::vec2 sample_scale = {Lerp(1.0f, 20.0f, sample_scale_x),
                                  Lerp(1.0f, 20.0f, sample_scale_y)};
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

  graph_builder_.DeclareConversion<std::tuple<Texture, Monotonic, Unitary>,
                                   std::tuple<Texture>>(
      "kaleidoscope",
      [this, texture_manager](
          std::tuple<Texture, Monotonic, Unitary> in) -> std::tuple<Texture> {
        auto& [in_tex, theta, fragment_count_scale] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = kaleidoscope->Activate();

        GlBindUniform(kaleidoscope, "model_transform", glm::mat4(1.0f));
        GlBindRenderTargetTextureToUniform(
            kaleidoscope, "in_tex", in_tex.RenderTarget(),
            gl::GlTextureBindingOptions{
                .sampling_mode = gl::GlTextureSamplingMode::kMirrorWrap});
        const int num_petals =
            static_cast<int>(Lerp(1.0f, 16.0f, fragment_count_scale));
        GL_BIND_LOCAL(kaleidoscope, theta);
        GL_BIND_LOCAL(kaleidoscope, num_petals);

        Rectangle().Draw();

        return std::make_tuple(tex);
      });

  graph_builder_.DeclareConversion<std::tuple<Texture, Monotonic, Monotonic>,
                                   std::tuple<Texture>>(
      "displace",
      [this, texture_manager](
          std::tuple<Texture, Monotonic, Monotonic> in) -> std::tuple<Texture> {
        auto& [in_tex, displacement_x, displacement_y] = in;
        Texture tex(width(), height(), texture_manager);

        auto rt_activation = tex.RenderTarget()->Activate();
        auto shader_activation = displace->Activate();

        GlBindUniform(displace, "model_transform", glm::mat4(1.0f));
        GlBindRenderTargetTextureToUniform(
            displace, "in_tex", in_tex.RenderTarget(),
            gl::GlTextureBindingOptions{
                .sampling_mode = gl::GlTextureSamplingMode::kMirrorWrap});
        const glm::vec2 displacement = {displacement_x, displacement_y};
        GL_BIND_LOCAL(displace, displacement);

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

GraphPreset::~GraphPreset() { ax::NodeEditor::DestroyEditor(editor_context_); }

absl::StatusOr<std::shared_ptr<Preset>> GraphPreset::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  RETURN_IF_ERROR(InitGlPrograms());
  return std::shared_ptr<GraphPreset>(new GraphPreset(texture_manager));
}

void GraphPreset::OnUpdateGeometry() { glViewport(0, 0, width(), height()); }

void GraphPreset::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  LOG_N_SEC(1.0, INFO) << absl::StrFormat("GraphPreset located at %X",
                                          reinterpret_cast<intptr_t>(this));
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
            state->energy(), state->bass_u(), state->mid_u(),
            state->treble_u()));
    Texture tex = std::get<0>(evaluation_graph_.Result<Texture>());

    {
      auto output_activation = output_render_target->Activate();
      Blit(tex);
    }
  }
}

}  // namespace opendrop
