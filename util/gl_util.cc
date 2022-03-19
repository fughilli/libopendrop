#include "util/gl_util.h"

#include "util/gl_helper.h"
#include "util/glm_helper.h"

namespace gl {

namespace {
// Parses the provided binding options and invokes the corresponding GL
// functions to configure the active texture as specified.
void ConfigureBindingOptions(GlTextureBindingOptions binding_options) {
  switch (binding_options.sampling_mode) {
    case GlTextureSamplingMode::kClamp:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      break;

    case GlTextureSamplingMode::kWrap:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      break;

    case GlTextureSamplingMode::kMirrorWrap:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
      break;

    case GlTextureSamplingMode::kClampToBorder:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
                       &binding_options.border_color[0]);
      break;
  }

  switch (binding_options.filtering_mode) {
    case GlTextureFilteringMode::kNearest:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      break;

    case GlTextureFilteringMode::kLinear:
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      break;
  }
}
}  // namespace

void GlBindRenderTargetTextureToUniform(
    std::shared_ptr<GlProgram> program, std::string texture_uniform_name,
    std::shared_ptr<GlRenderTarget> render_target,
    GlTextureBindingOptions binding_options) {
  glActiveTexture(GL_TEXTURE0 + render_target->texture_unit());
  glBindTexture(GL_TEXTURE_2D, render_target->texture_handle());

  ConfigureBindingOptions(binding_options);

  glUniform1i(glGetUniformLocation(program->program_handle(),
                                   texture_uniform_name.c_str()),
              render_target->texture_unit());
}

#define DEFINE_BIND_UNIFORM(type, uniform_func, value_expr)                    \
  void GlBindUniform(std::shared_ptr<GlProgram> program,                       \
                     std::string uniform_name, type value) {                   \
    uniform_func(                                                              \
        glGetUniformLocation(program->program_handle(), uniform_name.c_str()), \
        value_expr);                                                           \
  }
#define DEFINE_BIND_UNIFORM_V(v_type, uniform_v_func, value_expr)              \
  void GlBindUniform(std::shared_ptr<GlProgram> program,                       \
                     std::string uniform_name, v_type value) {                   \
    uniform_v_func(                                                            \
        glGetUniformLocation(program->program_handle(), uniform_name.c_str()), \
        1, value_expr);                                                        \
  }
#define DEFINE_BIND_UNIFORM_M(m_type, uniform_m_func, value_expr)              \
  void GlBindUniform(std::shared_ptr<GlProgram> program,                       \
                     std::string uniform_name, m_type value) {                   \
    uniform_m_func(                                                            \
        glGetUniformLocation(program->program_handle(), uniform_name.c_str()), \
        1, GL_FALSE, value_expr);                                              \
  }

DEFINE_BIND_UNIFORM(float, glUniform1f, value);
DEFINE_BIND_UNIFORM(int, glUniform1i, value);
DEFINE_BIND_UNIFORM(bool, glUniform1i, value);
DEFINE_BIND_UNIFORM_V(glm::vec2, glUniform2fv, &value.x);
DEFINE_BIND_UNIFORM_V(glm::vec3, glUniform3fv, &value.x);
DEFINE_BIND_UNIFORM_V(glm::vec4, glUniform4fv, &value.x);
DEFINE_BIND_UNIFORM_V(glm::ivec2, glUniform2iv, &value.x);
DEFINE_BIND_UNIFORM_V(glm::ivec3, glUniform3iv, &value.x);
DEFINE_BIND_UNIFORM_V(glm::ivec4, glUniform3iv, &value.x);
DEFINE_BIND_UNIFORM_M(glm::mat3, glUniformMatrix3fv, &value[0][0]);
DEFINE_BIND_UNIFORM_M(glm::mat4, glUniformMatrix4fv, &value[0][0]);

}  // namespace gl
