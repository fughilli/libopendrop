#ifndef GL_RENDER_TARGET_H_
#define GL_RENDER_TARGET_H_

#include <memory>
#include <mutex>

#include "absl/status/statusor.h"
#include "libopendrop/gl_interface.h"
#include "libopendrop/gl_texture_manager.h"

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
  static absl::StatusOr<std::shared_ptr<GlRenderTarget>> MakeShared(
      int width, int height, std::shared_ptr<GlTextureManager> texture_manager);
  virtual ~GlRenderTarget();

  virtual std::shared_ptr<GlRenderTargetActivation> Activate();

  void UpdateGeometry(int width, int height);

  int texture_unit() const { return texture_unit_; }
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

  bool swap_texture_unit(GlRenderTarget* other);

 private:
  GlRenderTarget(int width, int height, int texture_unit,
                 std::shared_ptr<GlTextureManager> texture_manager);

  std::mutex render_target_mu_;
  int width_, height_;
  int texture_unit_;
  unsigned int framebuffer_handle_;
  unsigned int renderbuffer_handle_;
  unsigned int texture_handle_;

  std::shared_ptr<GlTextureManager> texture_manager_;
};

}  // namespace gl

#endif  // GL_RENDER_TARGET_H_
