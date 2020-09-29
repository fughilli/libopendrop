#include "libopendrop/util/gl_util.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace gl {

void GlBindRenderTargetTextureToUniform(
    std::shared_ptr<GlProgram> program, std::string texture_uniform_name,
    std::shared_ptr<GlRenderTarget> render_target) {
  glActiveTexture(GL_TEXTURE0 + render_target->texture_unit());
  glBindTexture(GL_TEXTURE_2D, render_target->texture_handle());
  glUniform1i(glGetUniformLocation(program->program_handle(),
                                   texture_uniform_name.c_str()),
              render_target->texture_unit());
}
}  // namespace gl
