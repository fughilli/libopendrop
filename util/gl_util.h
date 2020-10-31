#ifndef LIBOPENDROP_UTIL_GL_UTIL_H_
#define LIBOPENDROP_UTIL_GL_UTIL_H_

#include <glm/vec4.hpp>
#include <memory>

#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"

namespace gl {

// Sampling mode for a bound texture.
enum class GlTextureSamplingMode {
  // Clamp UVs that fall outside of the texture boundaries to the boundary.
  kClamp = 0,
  // Wrap UVs that fall outside of the texture boundaries as if sampling from an
  // infinitely tiled texture in all directions.
  kWrap,
  // Similar to `kWrap`, but as if each alternating column is mirrored
  // left-to-right, and each alternating row is mirrored top-to-bottom.
  kMirrorWrap,
  // UVs that fall outside of the texture boundaries sample a separate border
  // color, instead of sampling the texture.
  kClampToBorder,
};

// Filtering mode for a bound texture.
enum class GlTextureFilteringMode {
  // Nearest-neighbor interpolation.
  kNearest = 0,
  // Linear interpolation between the four closest pixels to the UV.
  kLinear,
};

// Aggregates sampling mode, filtering mode, and border color.
struct GlTextureBindingOptions {
  GlTextureSamplingMode sampling_mode = GlTextureSamplingMode::kClamp;
  GlTextureFilteringMode filtering_mode = GlTextureFilteringMode::kLinear;
  glm::vec4 border_color = glm::vec4(0, 0, 0, 0);
};

// Binds the texture backing a gl::GlRenderTarget to a sampler uniform in a
// gl::GlProgram. Configures the bound texture with the provided binding
// options.
void GlBindRenderTargetTextureToUniform(
    std::shared_ptr<GlProgram> program, std::string texture_uniform_name,
    std::shared_ptr<GlRenderTarget> render_target,
    GlTextureBindingOptions binding_options);

}  // namespace gl

#endif  // LIBOPENDROP_UTIL_GL_UTIL_H_
