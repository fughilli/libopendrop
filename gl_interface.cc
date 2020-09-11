#include "gl_interface.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <iostream>

namespace gl {

namespace {
constexpr int kErrorStringMaxLength = 1024;

unsigned int GetGlShaderType(GlShaderType type) {
  switch (type) {
    case GlShaderType::kVertex:
      return GL_VERTEX_SHADER;
    case GlShaderType::kFragment:
      return GL_FRAGMENT_SHADER;
    default:
      return 0;
  }
}
}  // namespace

GlShader::GlShader(GlShaderType type, std::string shader_text) {
  shader_handle_ = glCreateShader(GetGlShaderType(type));
  const char* string_pointers[] = {shader_text.c_str()};
  glShaderSource(shader_handle_, 1, string_pointers, nullptr);
}

GlShader::~GlShader() { glDeleteShader(shader_handle_); }

bool GlShader::Compile(std::string* error_text) const {
  glCompileShader(shader_handle_);
  int success = 0;
  glGetShaderiv(shader_handle_, GL_COMPILE_STATUS, &success);
  if (!success) {
    if (error_text == nullptr) {
      return false;
    }
    char error_text_buffer[kErrorStringMaxLength];
    glGetShaderInfoLog(shader_handle_, kErrorStringMaxLength, nullptr,
                       error_text_buffer);
    error_text->append(error_text_buffer);
    return false;
  }
  return true;
}

GlProgram::GlProgram() { program_handle_ = glCreateProgram(); }

GlProgram::~GlProgram() { glDeleteProgram(program_handle_); }

const GlProgram& GlProgram::Attach(const GlShader& shader) const {
  glAttachShader(program_handle_, shader.GetHandle());
  return *this;
}

bool GlProgram::Link(std::string* error_text) const {
  glLinkProgram(program_handle_);
  int success = 0;
  glGetProgramiv(program_handle_, GL_LINK_STATUS, &success);
  if (!success) {
    if (error_text == nullptr) {
      return false;
    }
    char error_text_buffer[kErrorStringMaxLength];
    glGetProgramInfoLog(program_handle_, kErrorStringMaxLength, nullptr,
                        error_text_buffer);
    error_text->append(error_text_buffer);
    return false;
  }
  return true;
}

void GlProgram::Use() const { glUseProgram(program_handle_); }

}  // namespace gl
