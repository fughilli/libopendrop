#include "util/graph/rendering.h"

#include <algorithm>
#include <memory>

#include "absl/types/span.h"
#include "graph.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_node_editor.h"
#include "util/container/algorithms.h"
#include "util/graph/graph.h"
#include "util/graph/types/texture.h"
#include "util/graph/types/types.h"

namespace opendrop {

namespace {
namespace NE = ax::NodeEditor;

int IdForNode(const Graph& graph, const std::shared_ptr<Node>& node) {
  if (node == graph.input_node()) return 1;
  if (node == graph.output_node()) return 2;
  return 3 + IndexOf<std::shared_ptr<Node>>(graph.nodes(), node);
}

struct LinkConfig {
  int in_pin_id;
  int out_pin_id;

  static LinkConfig FromEdge(const Graph& graph, Edge edge) {
    return {.in_pin_id = IdForNode(graph, edge.out.node.lock()) * 1000 + 100 +
                         edge.out.index,
            .out_pin_id =
                IdForNode(graph, edge.in.node.lock()) * 1000 + edge.in.index};
  }
};

ImU32 BoolColor(bool ok) {
  if (ok) return IM_COL32(100, 255, 100, 255);
  return IM_COL32(255, 100, 100, 255);
}

void RenderPinContent(int index, const OpaqueTuple& tuple) {
  const Type type = tuple.Types()[index];
  switch (type) {
    case Type::kTexture: {
      const Texture& texture = tuple.Ref<Texture>(index);

      if (texture.RenderTarget() == nullptr) break;

      const size_t width = texture.width();
      const size_t height = texture.height();
      const auto [x_scale, y_scale] =
          width > height
              ? std::make_tuple(ImVec2(0, 1),
                                ImVec2(static_cast<float>(height) / width, 0))
              : std::make_tuple(ImVec2(0, static_cast<float>(width) / height),
                                ImVec2(1, 0));
      ImGui::Image((ImTextureID)texture.RenderTarget()->texture_handle(),
                   ImVec2(50, 50), x_scale, y_scale);
      LOG(INFO) << "Rendering texture at handle "
                << texture.RenderTarget()->texture_handle();
    } break;
    case Type::kUnitary: {
      ImGui::Text("(%0.4f)", static_cast<float>(tuple.Ref<Unitary>(index)));
    } break;
    default:
      break;
  }
}

}  // namespace

void RenderNode(ax::NodeEditor::EditorContext* context, const Node& node,
                int id, int evaluation_index) {
  NE::BeginNode(id);
  if (node.conversion != nullptr) {
    ImGui::Text("%s", node.conversion->name.c_str());
  } else {
    ImGui::Text("IO");
  }

  ImGui::Text("#%d", evaluation_index);

  for (size_t i = 0; i < std::max(node.input_tuple.Types().size(),
                                  node.output_tuple.Types().size());
       ++i) {
    if (i < node.input_tuple.Types().size()) {
      Type input_type = node.input_tuple.Types()[i];
      NE::BeginPin(id * 1000 + 100 + i, NE::PinKind::Input);
      ImGui::PushStyleColor(ImGuiCol_Text,
                            BoolColor(!node.input_tuple.CellIsEmpty(i)));
      ImGui::Text("-> %s", ToString(input_type).c_str());
      ImGui::PopStyleColor();
      NE::EndPin();
    }
    if (i < node.output_tuple.Types().size()) {
      Type output_type = node.output_tuple.Types()[i];
      if (i < node.input_tuple.Types().size()) {
        ImGui::SameLine();
      }
      NE::BeginPin(id * 1000 + i, NE::PinKind::Output);
      ImGui::PushStyleColor(ImGuiCol_Text,
                            BoolColor(!node.output_tuple.CellIsEmpty(i)));
      ImGui::Text("%s ->", ToString(output_type).c_str());
      ImGui::PopStyleColor();
      RenderPinContent(i, node.output_tuple);
      NE::EndPin();
    }
  }

  NE::EndNode();
}

void RenderInputEdges(ax::NodeEditor::EditorContext* context,
                      const Graph& graph, const Node& node, int id) {
  for (int i = 0; i < node.input_edges.size(); ++i) {
    Edge input_edge = node.input_edges[i];
    auto link_config = LinkConfig::FromEdge(graph, input_edge);
    NE::Link(id * 1000 + 200 * i, link_config.in_pin_id,
             link_config.out_pin_id);
  }
}

void RenderGraph(ax::NodeEditor::EditorContext* context, const Graph& graph) {
  NE::SetCurrentEditor(context);

  NE::Begin("Graph Viewer", ImVec2(0.0f, 0.0f));

  if (graph.valid()) {
    for (const auto& node : graph.nodes()) {
      int evaluation_index = -1;
      std::vector<std::shared_ptr<Node>> nodes_in_evaluation_order =
          graph.NodesInEvaluationOrder();
      if (std::find(nodes_in_evaluation_order.begin(),
                    nodes_in_evaluation_order.end(),
                    node) != nodes_in_evaluation_order.end()) {
        evaluation_index = IndexOf<std::shared_ptr<Node>>(
            graph.NodesInEvaluationOrder(), node);
      }
      RenderNode(context, *node, IdForNode(graph, node), evaluation_index);
    }
    RenderNode(context, *graph.input_node(),
               IdForNode(graph, graph.input_node()), -1);
    RenderNode(context, *graph.output_node(),
               IdForNode(graph, graph.output_node()), -1);

    for (const auto& node : graph.nodes()) {
      RenderInputEdges(context, graph, *node, IdForNode(graph, node));
    }
    RenderInputEdges(context, graph, *graph.output_node(),
                     IdForNode(graph, graph.output_node()));
  }

  NE::End();

  NE::SetCurrentEditor(nullptr);
}

}  // namespace opendrop
