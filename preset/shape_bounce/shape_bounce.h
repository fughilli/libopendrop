#ifndef PRESET_SHAPE_BOUNCE_SHAPE_BOUNCE_H_
#define PRESET_SHAPE_BOUNCE_SHAPE_BOUNCE_H_

#include <vector>

#include "absl/status/statusor.h"
#include "gl_interface.h"
#include "gl_render_target.h"
#include "gl_texture_manager.h"
#include "preset/preset.h"
#include "primitive/ngon.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "util/filter.h"
#include "util/glm_helper.h"

namespace opendrop {

class ShapeBounce : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "ShapeBounce"; }

 protected:
  ShapeBounce(std::shared_ptr<gl::GlProgram> warp_program,
              std::shared_ptr<gl::GlProgram> composite_program,
              std::shared_ptr<gl::GlProgram> ngon_program,
              std::shared_ptr<gl::GlRenderTarget> front_render_target,
              std::shared_ptr<gl::GlRenderTarget> back_render_target,
              std::shared_ptr<gl::GlTextureManager> texture_manager, int n);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlProgram> ngon_program_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;

  Rectangle rectangle_;
  Ngon ngon_;

  glm::vec2 velocity_{0, 0};
  glm::vec2 position_{0, 0};

  std::shared_ptr<IirFilter> bass_filter_;
  std::shared_ptr<HystereticMapFilter> bass_power_filter_;
};

}  // namespace opendrop

#endif  // PRESET_SHAPE_BOUNCE_SHAPE_BOUNCE_H_
