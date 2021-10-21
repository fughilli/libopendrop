#include "libopendrop/util/gl_util.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

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

void GlBindUniform(std::shared_ptr<GlProgram> program, std::string uniform_name,
                   float value) {
  glUniform1f(
      glGetUniformLocation(program->program_handle(), uniform_name.c_str()),
      value);
}

void GlBindUniform(std::shared_ptr<GlProgram> program, std::string uniform_name,
                   glm::vec4 value) {
  glUniform4fv(
      glGetUniformLocation(program->program_handle(), uniform_name.c_str()), 1,
      &value.x);
}

void GlBindUniform(std::shared_ptr<GlProgram> program, std::string uniform_name,
                   glm::mat4 value) {
  glUniformMatrix4fv(
      glGetUniformLocation(program->program_handle(), uniform_name.c_str()), 1,
      GL_FALSE, &value[0][0]);
}

void GlBindUniform(std::shared_ptr<GlProgram> program, std::string uniform_name,
                   glm::ivec2 value) {
  glUniform2iv(
      glGetUniformLocation(program->program_handle(), uniform_name.c_str()), 1,
      &value.x);
}

void GlBindUniform(std::shared_ptr<GlProgram> program, std::string uniform_name,
                   int value) {
  glUniform1i(
      glGetUniformLocation(program->program_handle(), uniform_name.c_str()),
      value);
}

}  // namespace gl
