#ifndef PRESET_THIN_FILM_THIN_FILM_H_
#define PRESET_THIN_FILM_THIN_FILM_H_

#include <vector>

#include "absl/status/statusor.h"
#include "preset/preset.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "third_party/glm_helper.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "util/signal/filter.h"

namespace opendrop {

class ThinFilm : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "ThinFilm"; }

 protected:
  ThinFilm(std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  std::shared_ptr<gl::GlProgram> thin_film_program_;

  Rectangle rectangle_;

  std::shared_ptr<IirFilter> rot_filter_ =
      IirSinglePoleFilter(0.01, IirSinglePoleFilterType::kLowpass);
};

}  // namespace opendrop

#endif  // PRESET_THIN_FILM_THIN_FILM_H_
