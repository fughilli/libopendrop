#ifndef UTIL_GRAPH_RENDERING_H_
#define UTIL_GRAPH_RENDERING_H_

#include "imgui_node_editor.h"
#include "util/graph/graph.h"

namespace opendrop {

void RenderConversion(ax::NodeEditor::EditorContext* context,
                      const Conversion& conversion, int id);

void RenderGraph(ax::NodeEditor::EditorContext* context, const Graph& graph);

}  // namespace opendrop

#endif  // UTIL_GRAPH_RENDERING_H_
