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
  if (node == graph.io_node) return 1;
  return 2 + IndexOf<std::shared_ptr<Node>>(graph.nodes, node);
}

struct LinkConfig {
  int in_pin_id;
  int out_pin_id;

  static LinkConfig FromEdge(const Graph& graph, Edge edge) {
    return {.out_pin_id = IdForNode(graph, edge.value_in.first) * 1000 +
                          edge.value_in.second,
            .in_pin_id = IdForNode(graph, edge.alias.first) * 1000 + 100 +
                         edge.alias.second};
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

  const size_t num_lines = std::max(node.InputSize(), node.OutputSize());

  for (size_t i = 0; i < num_lines; ++i) {
    if (i < node.input_tuple.Types().size()) {
      Type input_type = node.input_tuple.Types()[i];
      NE::BeginPin(id * 1000 + 100 + i, NE::PinKind::Input);
      ImGui::PushStyleColor(ImGuiCol_Text,
                            BoolColor(!node.input_tuple.CellIsEmpty(i)));
      ImGui::Text("-> %s", ToString(input_type).c_str());
      ImGui::PopStyleColor();
      NE::EndPin();
    }
    ImGui::SameLine();
    if (i < node.output_tuple.Types().size()) {
      Type output_type = node.output_tuple.Types()[i];
      NE::BeginPin(id * 1000 + i, NE::PinKind::Output);
      ImGui::PushStyleColor(ImGuiCol_Text,
                            BoolColor(!node.output_tuple.CellIsEmpty(i)));
      ImGui::Text("%s ->", ToString(output_type).c_str());
      ImGui::PopStyleColor();
      NE::EndPin();
    }
  }

  NE::EndNode();
}

void RenderInputEdges(ax::NodeEditor::EditorContext* context,
                      const Graph& graph, const Node& node, int id) {
  for (Edge input_edge : node.input_edges) {
    auto link_config = LinkConfig::FromEdge(graph, input_edge);
    NE::Link(id * 1000 + 200, link_config.in_pin_id, link_config.out_pin_id);
  }
}

void RenderGraph(ax::NodeEditor::EditorContext* context, const Graph& graph) {
  NE::SetCurrentEditor(context);

  NE::Begin("Graph Viewer", ImVec2(0.0f, 0.0f));

  for (const auto& node : graph.nodes) {
    int evaluation_index = -1;
    std::vector<std::shared_ptr<Node>> nodes_in_evaluation_order =
        graph.NodesInEvaluationOrder();
    if (std::find(nodes_in_evaluation_order.begin(),
                  nodes_in_evaluation_order.end(),
                  node) != nodes_in_evaluation_order.end()) {
      evaluation_index =
          IndexOf<std::shared_ptr<Node>>(graph.NodesInEvaluationOrder(), node);
    }
    RenderNode(context, *node, IdForNode(graph, node), evaluation_index);
  }
  RenderNode(context, *graph.io_node, IdForNode(graph, graph.io_node), -1);

  for (const auto& node : graph.nodes) {
    RenderInputEdges(context, graph, *node, IdForNode(graph, node));
  }
  RenderInputEdges(context, graph, *graph.io_node,
                   IdForNode(graph, graph.io_node));

  NE::End();

  NE::SetCurrentEditor(nullptr);
}

}  // namespace opendrop
