#ifndef UTIL_GRAPHICS_GL_RENDER_TARGET_H_
#define UTIL_GRAPHICS_GL_RENDER_TARGET_H_

#include <memory>
#include <mutex>

#include "absl/status/statusor.h"
#include "third_party/glm_helper.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_texture_manager.h"

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
  // Construction options for GlRenderTarget.
  struct Options {
    // Whether or not to support a depth buffer for the render target. When
    // `true`, there will be a depth buffer attachment and associated texture
    // allocated for the render target.
    bool enable_depth = false;
  };

  static absl::StatusOr<std::shared_ptr<GlRenderTarget>> MakeShared(
      int width, int height, std::shared_ptr<GlTextureManager> texture_manager,
      Options options);
  static absl::StatusOr<std::shared_ptr<GlRenderTarget>> MakeShared(
      int width, int height,
      std::shared_ptr<GlTextureManager> texture_manager) {
    return MakeShared(width, height, texture_manager, Options());
  }
  virtual ~GlRenderTarget();

  virtual std::shared_ptr<GlRenderTargetActivation> Activate();

  void UpdateGeometry(int width, int height);

  int texture_unit() const { return texture_unit_; }
  unsigned int renderbuffer_handle() const { return renderbuffer_handle_; }
  unsigned int framebuffer_handle() const { return framebuffer_handle_; }
  unsigned int texture_handle() const { return texture_handle_; }
  unsigned int depth_buffer_handle() const { return depth_buffer_handle_; }
  const Options& options() const { return options_; }
  int width() {
    std::unique_lock<std::mutex> lock(render_target_mu_);
    return width_;
  }
  int height() {
    std::unique_lock<std::mutex> lock(render_target_mu_);
    return height_;
  }
  glm::ivec2 size() {
    std::unique_lock<std::mutex> lock(render_target_mu_);
    return {width_, height_};
  }

  bool swap_texture_unit(GlRenderTarget* other);

 private:
  GlRenderTarget(int width, int height, int texture_unit,
                 std::shared_ptr<GlTextureManager> texture_manager,
                 Options options);

  std::mutex render_target_mu_;
  int width_, height_;
  int texture_unit_;
  unsigned int framebuffer_handle_;
  unsigned int renderbuffer_handle_;
  unsigned int texture_handle_;
  unsigned int depth_buffer_handle_;

  std::shared_ptr<GlTextureManager> texture_manager_;
  const Options options_;
};

}  // namespace gl

#endif  // UTIL_GRAPHICS_GL_RENDER_TARGET_H_
