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
  if (node == graph.io_node) return 0;
  return 1 + IndexOf<std::shared_ptr<Node>>(graph.nodes, node);
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

}  // namespace

void RenderNode(ax::NodeEditor::EditorContext* context, const Node& node,
                int id) {
  NE::BeginNode(id);
  ImGui::Text("%s", node.conversion->name.c_str());

  const size_t num_lines = std::max(node.conversion->InputTypes().size(),
                                    node.conversion->OutputTypes().size());

  for (size_t i = 0; i < num_lines; ++i) {
    if (i < node.conversion->InputTypes().size()) {
      Type input_type = node.conversion->InputTypes()[i];
      NE::BeginPin(id * 1000 + 100 + i, NE::PinKind::Input);
      ImGui::Text("-> %s", ToString(input_type).c_str());
      NE::EndPin();
    }
    ImGui::SameLine();
    if (i < node.conversion->OutputTypes().size()) {
      Type output_type = node.conversion->OutputTypes()[i];
      NE::BeginPin(id * 1000 + i, NE::PinKind::Output);
      ImGui::Text("%s ->", ToString(output_type).c_str());
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
    RenderNode(context, *node, IdForNode(graph, node));
  }

  for (const auto& node : graph.nodes) {
    RenderInputEdges(context, graph, *node, IdForNode(graph, node));
  }
  RenderInputEdges(context, graph, *graph.io_node, 0);

  NE::End();

  NE::SetCurrentEditor(nullptr);
}

}  // namespace opendrop
