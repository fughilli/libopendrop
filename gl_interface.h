#ifndef GL_INTERFACE_H_
#define GL_INTERFACE_H_

#include <memory>

namespace gl {

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
