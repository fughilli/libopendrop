#ifndef PRESET_CUBE_BOOM_CUBE_BOOM_H_
#define PRESET_CUBE_BOOM_CUBE_BOOM_H_

#include <vector>

#include "absl/status/statusor.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "preset/preset.h"
#include "primitive/model.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"

namespace opendrop {

class CubeBoom : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "CubeBoom"; }

  int max_count() const override { return 1; }

 protected:
  CubeBoom(std::shared_ptr<gl::GlProgram> warp_program,
           std::shared_ptr<gl::GlProgram> composite_program,
           std::shared_ptr<gl::GlProgram> model_program,
           std::shared_ptr<gl::GlRenderTarget> front_render_target,
           std::shared_ptr<gl::GlRenderTarget> back_render_target,
           std::shared_ptr<gl::GlRenderTarget> depth_output_target,
           std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlProgram> model_program_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;
  std::shared_ptr<gl::GlRenderTarget> depth_output_target_;

  std::vector<glm::vec2> vertices_;
  Rectangle rectangle_;
  Polyline polyline_;
  Model cube_;
  Model monkey_;
  Model shrek_;
};

}  // namespace opendrop

#endif  // PRESET_CUBE_BOOM_CUBE_BOOM_H_
