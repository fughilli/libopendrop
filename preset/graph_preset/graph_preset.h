#ifndef PRESET_GRAPH_PRESET_GRAPH_PRESET_H_
#define PRESET_GRAPH_PRESET_GRAPH_PRESET_H_

#include <vector>

#include "absl/status/statusor.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "preset/preset.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "third_party/glm_helper.h"
#include "util/graph/graph.h"
#include "imgui_node_editor.h"

namespace opendrop {

class GraphPreset : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "GraphPreset"; }

  ~GraphPreset() override;

 protected:
  GraphPreset(
                 std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  GraphBuilder graph_builder_;
  Graph evaluation_graph_;

  ax::NodeEditor::EditorContext* editor_context_;
};

}  // namespace opendrop

#endif  // PRESET_GRAPH_PRESET_GRAPH_PRESET_H_
