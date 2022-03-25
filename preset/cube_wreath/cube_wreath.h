#ifndef PRESET_CUBE_WREATH_CUBE_WREATH_H_
#define PRESET_CUBE_WREATH_CUBE_WREATH_H_

#include <vector>

#include "util/glm_helper.h"
#include "absl/status/statusor.h"
#include "gl_interface.h"
#include "gl_render_target.h"
#include "gl_texture_manager.h"
#include "preset/common/outline_model.h"
#include "preset/preset.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"

namespace opendrop {

class CubeWreath : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "CubeWreath"; }

  int max_count() const override { return 1; }

 protected:
  CubeWreath(std::shared_ptr<gl::GlProgram> warp_program,
             std::shared_ptr<gl::GlProgram> composite_program,
             std::shared_ptr<gl::GlProgram> passthrough_program,
             std::shared_ptr<gl::GlRenderTarget> model_texture_target,
             std::shared_ptr<gl::GlRenderTarget> front_render_target,
             std::shared_ptr<gl::GlRenderTarget> back_render_target,
             std::shared_ptr<gl::GlRenderTarget> depth_output_target,
             std::shared_ptr<OutlineModel> outline_model,
             std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  void DrawCubes(float power, float energy, glm::vec3 zoom_vec, int num_cubes);

  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlProgram> passthrough_program_;
  std::shared_ptr<gl::GlRenderTarget> model_texture_target_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;
  std::shared_ptr<gl::GlRenderTarget> depth_output_target_;
  std::shared_ptr<OutlineModel> outline_model_;

  std::vector<glm::vec2> vertices_;
  Rectangle rectangle_;
  Polyline polyline_;

  float background_hue_ = 0;
  bool texture_trigger_ = false;

  float rot_arg_ = 0.0f;
};

}  // namespace opendrop

#endif  // PRESET_CUBE_WREATH_CUBE_WREATH_H_
