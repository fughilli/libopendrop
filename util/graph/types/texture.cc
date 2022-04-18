#include "util/graph/types/texture.h"

#include <ostream>

#include "absl/strings/str_format.h"
#include "primitive/rectangle.h"
#include "shader/blit.fsh.h"
#include "shader/blit.vsh.h"
#include "third_party/gl_helper.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"

namespace opendrop {

namespace {

std::shared_ptr<gl::GlProgram> blit_program = nullptr;

std::shared_ptr<gl::GlProgram> GetBlitProgram() {
  if (blit_program != nullptr) return blit_program;

  absl::StatusOr<std::shared_ptr<gl::GlProgram>> status_or_blit_program =
      gl::GlProgram::MakeShared(blit_vsh::Code(), blit_fsh::Code());

  blit_program = std::move(status_or_blit_program).value();
  return blit_program;
}
}  // namespace

std::ostream& operator<<(std::ostream& os, const Texture& texture) {
  return os << absl::StrFormat("Texture(%s)", ToString(texture.Color()));
}

void Blit(const Texture& texture) {
  GetBlitProgram()->Use();
  // Bind the source texture and alpha value.
  GlBindRenderTargetTextureToUniform(GetBlitProgram(), "source_texture",
                                     texture.RenderTarget(),
                                     gl::GlTextureBindingOptions());
  glUniform1f(glGetUniformLocation(GetBlitProgram()->program_handle(), "alpha"),
              1.0f);

  Rectangle().Draw();
}

}  // namespace opendrop
