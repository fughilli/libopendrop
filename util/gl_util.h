#ifndef LIBOPENDROP_UTIL_GL_UTIL_H_
#define LIBOPENDROP_UTIL_GL_UTIL_H_

#include <memory>

#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_render_target.h"

namespace gl {

// Binds the texture backing a gl::GlRenderTarget to a sampler uniform in a
// gl::GlProgram.
void GlBindRenderTargetTextureToUniform(
    std::shared_ptr<GlProgram> program, std::string texture_uniform_name,
    std::shared_ptr<GlRenderTarget> render_target);

}  // namespace gl

#endif  // LIBOPENDROP_UTIL_GL_UTIL_H_
