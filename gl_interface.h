#ifndef GL_INTERFACE_H_
#define GL_INTERFACE_H_

#include <memory>
#include <string>

#include "absl/status/statusor.h"

namespace gl {

enum class GlShaderType : int {
  kVertex = 0,
  kFragment = 1,
  kGeometry = 2,
  kCompute = 3,
};

class GlShader {
 public:
  GlShader(GlShaderType type, std::string shader_text);
  ~GlShader();
  bool Compile(std::string* error_text) const;
  unsigned int GetHandle() const { return shader_handle_; }

 private:
  unsigned int shader_handle_;
};

class GlProgram {
 public:
  GlProgram();
  ~GlProgram();

  const GlProgram& Attach(const GlShader& shader) const;

  bool Link(std::string* error_text) const;
  void Use() const;

  unsigned int program_handle() const { return program_handle_; }

  static absl::StatusOr<std::shared_ptr<GlProgram>> MakeShared(
      std::string vertex_code, std::string fragment_code);

 private:
  unsigned int program_handle_;
};

class GlContextActivation {
 public:
  virtual ~GlContextActivation() {}
};

class GlContext {
 public:
  virtual ~GlContext() {}
  virtual std::shared_ptr<GlContextActivation> Activate() = 0;
};

class GlInterface {
 public:
  virtual std::shared_ptr<GlContext> AllocateSharedContext() = 0;
  virtual void SetVsync(bool enable) = 0;

  // Swaps double-buffers.
  virtual void SwapBuffers() = 0;
};

};  // namespace gl

#endif  // GL_INTERFACE_H_
