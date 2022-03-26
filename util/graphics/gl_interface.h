#ifndef GL_INTERFACE_H_
#define GL_INTERFACE_H_

#include <memory>
#include <string>

#include "absl/status/statusor.h"
#include "third_party/glm_helper.h"

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

class GlProgram;

// Represents an active shader program. Constructing such an object caches the
// current program index and invokes glUseProgram() with the program index to
// which the activation corresponds. The old program index is restored on
// destruction.
class GlProgramActivation {
 public:
  GlProgramActivation(std::shared_ptr<const GlProgram> program);
  virtual ~GlProgramActivation();

 private:
  int old_program_;
  std::shared_ptr<const GlProgram> program_;
};

class GlProgram : public std::enable_shared_from_this<GlProgram> {
 public:
  GlProgram();
  ~GlProgram();

  const GlProgram& Attach(const GlShader& shader) const;

  bool Link(std::string* error_text) const;
  void Use() const;

  unsigned int program_handle() const { return program_handle_; }

  static absl::StatusOr<std::shared_ptr<GlProgram>> MakeShared(
      std::string vertex_code, std::string fragment_code);

  std::shared_ptr<GlProgramActivation> Activate() const;

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
  virtual glm::ivec2 DrawableSize() = 0;

  // Swaps double-buffers.
  virtual void SwapBuffers() = 0;
};

};  // namespace gl

#endif  // GL_INTERFACE_H_
