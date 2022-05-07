#ifndef UTIL_GRAPHICS_GL_UTIL_H_
#define UTIL_GRAPHICS_GL_UTIL_H_

#include <memory>

#include "third_party/glm_helper.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"

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

void GlClear(glm::vec4 color);

// Binds the texture backing a gl::GlRenderTarget to a sampler uniform in a
// gl::GlProgram. Configures the bound texture with the provided binding
// options.
void GlBindRenderTargetTextureToUniform(
    std::shared_ptr<GlProgram> program, std::string texture_uniform_name,
    std::shared_ptr<GlRenderTarget> render_target,
    GlTextureBindingOptions binding_options);

// Binds a value by name in a gl::GlProgram.
#define DECLARE_BIND_UNIFORM(type)                       \
  void GlBindUniform(std::shared_ptr<GlProgram> program, \
                     std::string uniform_name, type value);

DECLARE_BIND_UNIFORM(float);
DECLARE_BIND_UNIFORM(int);
DECLARE_BIND_UNIFORM(bool);
DECLARE_BIND_UNIFORM(glm::vec2);
DECLARE_BIND_UNIFORM(glm::vec3);
DECLARE_BIND_UNIFORM(glm::vec4);
DECLARE_BIND_UNIFORM(glm::ivec2);
DECLARE_BIND_UNIFORM(glm::ivec3);
DECLARE_BIND_UNIFORM(glm::ivec4);
DECLARE_BIND_UNIFORM(glm::mat3);
DECLARE_BIND_UNIFORM(glm::mat4);

#undef DECLARE_BIND_UNIFORM

#define GL_BIND_LOCAL(program, value) \
  GlBindUniform(program, "" #value "", value)

}  // namespace gl

#endif  // UTIL_GRAPHICS_GL_UTIL_H_
