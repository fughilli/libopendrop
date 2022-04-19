#include "util/graph/rendering.h"

#include <algorithm>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_node_editor.h"

namespace opendrop {

namespace {
namespace NE = ax::NodeEditor;

constexpr int kMaxIoPerConversion = 100;
}  // namespace

void RenderConversion(ax::NodeEditor::EditorContext* context,
                      const Conversion& conversion, int id) {
  NE::BeginNode(id++);
  ImGui::Text("%s", conversion.name.c_str());

  const size_t num_lines =
      std::max(conversion.InputTypes().size(), conversion.OutputTypes().size());

  for (size_t i = 0; i < num_lines; ++i) {
    if (i < conversion.InputTypes().size()) {
      Type input_type = conversion.InputTypes()[i];
      NE::BeginPin(id++, NE::PinKind::Input);
      ImGui::Text("-> %s", ToString(input_type).c_str());
      NE::EndPin();
    }
    ImGui::SameLine();
    if (i < conversion.OutputTypes().size()) {
      Type output_type = conversion.OutputTypes()[i];
      NE::BeginPin(id++, NE::PinKind::Output);
      ImGui::Text("%s ->", ToString(output_type).c_str());
      NE::EndPin();
    }
  }

  NE::EndNode();
}

void RenderGraph(ax::NodeEditor::EditorContext* context, const Graph& graph) {
  NE::SetCurrentEditor(context);

  NE::Begin("Graph Viewer", ImVec2(0.0f, 0.0f));

  int id = 1;
  for (const auto& conversion : graph.conversions) {
    RenderConversion(context, *conversion, id);

    id += kMaxIoPerConversion;
  }

  NE::End();

  NE::SetCurrentEditor(nullptr);
}

}  // namespace opendrop
