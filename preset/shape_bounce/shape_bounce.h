#ifndef PRESETS_SHAPE_BOUNCE_SHAPE_BOUNCE_H_
#define PRESETS_SHAPE_BOUNCE_SHAPE_BOUNCE_H_

#include <glm/vec2.hpp>
#include <vector>

#include "absl/status/statusor.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"
#include "libopendrop/gl_texture_manager.h"
#include "libopendrop/preset/preset.h"
#include "libopendrop/primitive/ngon.h"
#include "libopendrop/primitive/polyline.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/filter.h"

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

  glm::vec2 velocity_;
  glm::vec2 position_;

  std::shared_ptr<IirFilter> bass_filter_;
  HystereticMapFilter bass_power_filter_;
};

}  // namespace opendrop

#endif  // PRESETS_SHAPE_BOUNCE_SHAPE_BOUNCE_H_
