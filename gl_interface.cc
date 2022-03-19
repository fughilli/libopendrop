#include "gl_interface.h"

#include <iostream>

#include "absl/strings/str_cat.h"
#include "util/gl_helper.h"
#include "util/logging.h"

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

GlProgramActivation::GlProgramActivation(
    std::shared_ptr<const GlProgram> program) {
  glGetIntegerv(GL_CURRENT_PROGRAM, &old_program_);
  glUseProgram(program->program_handle());
}

GlProgramActivation::~GlProgramActivation() { glUseProgram(old_program_); }

std::shared_ptr<GlProgramActivation> GlProgram::Activate() const {
  return std::make_shared<GlProgramActivation>(shared_from_this());
}

absl::StatusOr<std::shared_ptr<gl::GlProgram>> GlProgram::MakeShared(
    std::string vertex_code, std::string fragment_code) {
  LOG(DEBUG) << "[Compiling program]\nVERTEX SHADER CODE:\n"
             << "========================================" << vertex_code
             << "========================================"
             << "\nFRAGMENT SHADER CODE:\n"
             << "========================================" << fragment_code
             << "========================================";
  auto gl_program = std::make_shared<gl::GlProgram>();
  gl::GlShader vertex_shader(gl::GlShaderType::kVertex, vertex_code);
  std::string error_string = "";
  if (!vertex_shader.Compile(&error_string)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Failed to compile vertex shader: ", error_string));
  }
  gl::GlShader fragment_shader(gl::GlShaderType::kFragment, fragment_code);
  if (!fragment_shader.Compile(&error_string)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Failed to compile fragment shader: ", error_string));
  }

  if (!gl_program->Attach(vertex_shader)
           .Attach(fragment_shader)
           .Link(&error_string)) {
    return absl::UnknownError(
        absl::StrCat("Failed to link program: ", error_string));
  }

  return gl_program;
}

}  // namespace gl
