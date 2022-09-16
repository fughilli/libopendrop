#ifndef PRESET_FLUID_PRESET_FLUID_PRESET_H_
#define PRESET_FLUID_PRESET_FLUID_PRESET_H_

#include <vector>

#include "absl/status/statusor.h"
#include "preset/preset.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "third_party/glm_helper.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"

namespace opendrop {

class FluidPreset : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "FluidPreset"; }

 protected:
  FluidPreset(std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  // curl, vorticity, divergence, clear, pressure, gradient subtract, advection, blit
  std::array<std::shared_ptr<gl::GlProgram>, 9> programs_;
  // dye, velocity, divergence, curl, pressure
  std::array<std::shared_ptr<gl::GlRenderTarget>, 5> render_targets_;

  // curl program:
  //   [size, velocity] -> curl
  // vorticity program:
  //   [size, velocity, curl, curl_coeff, dt] -> velocity
  // divergence program:
  //   [size, velocity] -> divergence
  // clear pressure tex
  // pressure program (50x):
  //   [size, divergence, prev_pressure] -> pressure
  // gradient subtract program:
  //   [size, pressure, prev_velocity] -> velocity
  // advection program:
  //   [size, velocity, prev_velocity, dissipation, dt] -> velocity
  //   [size, velocity, prev_dye, dissipation, dt] -> dye

  std::vector<glm::vec2> vertices_;
  Polyline polyline_;
};

}  // namespace opendrop

#endif  // PRESET_FLUID_PRESET_FLUID_PRESET_H_
