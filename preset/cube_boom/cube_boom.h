#ifndef PRESETS_CUBE_BOOM_CUBE_BOOM_H_
#define PRESETS_CUBE_BOOM_CUBE_BOOM_H_

#include <glm/vec2.hpp>
#include <vector>

#include "absl/status/statusor.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/gl_texture_manager.h"
#include "libopendrop/preset/preset.h"
#include "libopendrop/primitive/model.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"

namespace opendrop {

class CubeBoom : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "CubeBoom"; }

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

#endif  // PRESETS_CUBE_BOOM_CUBE_BOOM_H_
