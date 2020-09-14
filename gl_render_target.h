#ifndef GL_RENDER_TARGET_H_
#define GL_RENDER_TARGET_H_

#include <memory>
#include <mutex>

#include "libopendrop/gl_interface.h"

namespace gl {

class GlRenderTarget;

class GlRenderTargetActivation {
 public:
  GlRenderTargetActivation(std::shared_ptr<GlRenderTarget> render_target);
  virtual ~GlRenderTargetActivation();

 private:
  std::shared_ptr<GlRenderTarget> render_target_;
};

class GlRenderTarget : public std::enable_shared_from_this<GlRenderTarget> {
 public:
  GlRenderTarget(int width, int height);
  virtual ~GlRenderTarget();

  virtual std::shared_ptr<GlRenderTargetActivation> Activate();

  void UpdateGeometry(int width, int height);

  unsigned int renderbuffer_handle() const { return renderbuffer_handle_; }
  unsigned int framebuffer_handle() const { return framebuffer_handle_; }
  unsigned int texture_handle() const { return texture_handle_; }
  int width() {
    std::unique_lock<std::mutex> lock(render_target_mu_);
    return width_;
  }
  int height() {
    std::unique_lock<std::mutex> lock(render_target_mu_);
    return height_;
  }

 private:
  std::mutex render_target_mu_;
  int width_, height_;
  unsigned int framebuffer_handle_;
  unsigned int renderbuffer_handle_;
  unsigned int texture_handle_;
};

}  // namespace gl

#endif  // GL_RENDER_TARGET_H_
