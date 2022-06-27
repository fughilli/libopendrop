#include "util/graph/rendering.h"

#include <algorithm>
#include <memory>

#include "absl/types/span.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_node_editor.h"
#include "util/container/algorithms.h"
#include "util/graph/graph.h"

namespace opendrop {

namespace {
namespace NE = ax::NodeEditor;

constexpr int kMaxIoPerConversion = 100;

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

  ImGui::Columns(2);

  for (size_t i = 0; i < node.input_tuple.Types().size(); ++i) {
    Type input_type = node.input_tuple.Types()[i];
    NE::BeginPin(id * 1000 + 100 + i, NE::PinKind::Input);
    ImGui::PushStyleColor(ImGuiCol_Text,
                          BoolColor(!node.input_tuple.CellIsEmpty(i)));
    ImGui::Text("-> %s", ToString(input_type).c_str());
    ImGui::PopStyleColor();
    NE::EndPin();
  }
  for (size_t i = 0; i < node.output_tuple.Types().size(); ++i) {
    Type output_type = node.output_tuple.Types()[i];
    NE::BeginPin(id * 1000 + i, NE::PinKind::Output);
    ImGui::PushStyleColor(ImGuiCol_Text,
                          BoolColor(!node.output_tuple.CellIsEmpty(i)));
    ImGui::Text("%s ->", ToString(output_type).c_str());
    ImGui::PopStyleColor();
    NE::EndPin();
  }

  ImGui::Columns();

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
