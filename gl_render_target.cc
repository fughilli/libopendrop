#include "libopendrop/gl_render_target.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <iostream>

#include "libopendrop/util/logging.h"
#include "libopendrop/util/status_macros.h"

namespace gl {

GlRenderTargetActivation::GlRenderTargetActivation(
    std::shared_ptr<GlRenderTarget> render_target)
    : render_target_(render_target) {
  // Configure the backing texture.
  glBindFramebuffer(GL_FRAMEBUFFER, render_target_->framebuffer_handle());
  glBindRenderbuffer(GL_RENDERBUFFER, render_target_->renderbuffer_handle());
  LOG(DEBUG) << "Bound framebuffer " << render_target_->framebuffer_handle()
             << " and renderbuffer " << render_target_->renderbuffer_handle();

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         render_target_->texture_handle(), 0);
  if (render_target_->options().enable_depth) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                           GL_TEXTURE_2D, render_target_->depth_buffer_handle(),
                           0);
  }

  LOG(DEBUG) << "Configured framebuffer texture as "
             << render_target_->texture_handle() << " for framebuffer "
             << render_target_->framebuffer_handle() << " and renderbuffer "
             << render_target_->renderbuffer_handle();
  auto framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (framebuffer_status == GL_FRAMEBUFFER_COMPLETE) {
    LOG(DEBUG) << "Successfully configured framebuffer";
  } else {
    LOG(ERROR) << "Failed to configure framebuffer";
    switch (framebuffer_status) {
      case GL_FRAMEBUFFER_UNDEFINED:
        LOG(ERROR) << "GL_FRAMEBUFFER_UNDEFINED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        LOG(ERROR) << "GL_FRAMEBUFFER_UNSUPPORTED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        LOG(ERROR) << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;
    }
    return;
  }

  glViewport(0, 0, render_target_->width(), render_target_->height());
  LOG(DEBUG) << "Configured viewport: " << render_target_->width() << "x"
             << render_target->height();
}

GlRenderTargetActivation::~GlRenderTargetActivation() {
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  LOG(DEBUG) << "Unbound framebuffer";
}

GlRenderTarget::GlRenderTarget(
    int width, int height, int texture_unit,
    std::shared_ptr<GlTextureManager> texture_manager, Options options)
    : texture_unit_(texture_unit),
      texture_manager_(texture_manager),
      options_(options) {
  // Create a new renderbuffer.
  glGenFramebuffers(1, &renderbuffer_handle_);
  LOG(DEBUG) << "Generated renderbuffer: " << renderbuffer_handle_;
  // Create a new framebuffer.
  glGenFramebuffers(1, &framebuffer_handle_);
  LOG(DEBUG) << "Generated framebuffer: " << framebuffer_handle_;
  // Generate a backing texture.
  glGenTextures(1, &texture_handle_);
  LOG(DEBUG) << "Generated texture: " << texture_handle_;

  if (options_.enable_depth) {
    glGenTextures(1, &depth_buffer_handle_);
  }

  UpdateGeometry(width, height);
}

absl::StatusOr<std::shared_ptr<GlRenderTarget>> GlRenderTarget::MakeShared(
    int width, int height, std::shared_ptr<GlTextureManager> texture_manager,
    Options options) {
  ASSIGN_OR_RETURN(auto texture_unit, texture_manager->Allocate());
  return std::shared_ptr<GlRenderTarget>(new GlRenderTarget(
      width, height, texture_unit, texture_manager, options));
}

GlRenderTarget::~GlRenderTarget() {
  LOG(DEBUG) << "Disposing render target";
  if (options_.enable_depth) {
    glDeleteTextures(1, &depth_buffer_handle_);
  }

  texture_manager_->Deallocate(texture_unit_);
  glDeleteTextures(1, &texture_handle_);
  glDeleteFramebuffers(1, &framebuffer_handle_);
  glDeleteFramebuffers(1, &renderbuffer_handle_);
}

void GlRenderTarget::UpdateGeometry(int width, int height) {
  std::unique_lock<std::mutex> lock(render_target_mu_);

  width_ = width;
  height_ = height;

  if (width_ == 0 || height_ == 0) {
    return;
  }

  glBindTexture(GL_TEXTURE_2D, texture_handle_);
  LOG(DEBUG) << "Bound RGBA texture: " << texture_handle_;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  if (options_.enable_depth) {
    glBindTexture(GL_TEXTURE_2D, depth_buffer_handle_);
    LOG(DEBUG) << "Bound depth texture: " << depth_buffer_handle_;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0,
                 GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  LOG(DEBUG) << "Unbound texture";
}

std::shared_ptr<GlRenderTargetActivation> GlRenderTarget::Activate() {
  CHECK(width_ != 0 && height_ != 0)
      << "Render target size must be nonzero before activation.";
  return std::make_shared<GlRenderTargetActivation>(shared_from_this());
}

bool GlRenderTarget::swap_texture_unit(GlRenderTarget* other) {
  std::unique_lock<std::mutex> lock(render_target_mu_);
  std::unique_lock<std::mutex> other_lock(other->render_target_mu_,
                                          std::defer_lock);
  if (!other_lock.try_lock()) {
    return false;
  }

  int intermediate_texture_handle = other->texture_handle_;
  other->texture_handle_ = texture_handle_;
  texture_handle_ = intermediate_texture_handle;
  return true;
}

}  // namespace gl
