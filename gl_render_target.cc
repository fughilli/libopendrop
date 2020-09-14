#include "libopendrop/gl_render_target.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <iostream>

#include "libopendrop/util/logging.h"

namespace gl {

GlRenderTargetActivation::GlRenderTargetActivation(
    std::shared_ptr<GlRenderTarget> render_target)
    : render_target_(render_target) {
  // Configure the backing texture.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target->texture_handle());
  LOG(DEBUG) << "Bound texture: " << render_target->texture_handle();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_target->width(),
               render_target->height(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  LOG(DEBUG) << "Applied texture properties";
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glBindTexture(GL_TEXTURE_2D, 0);
  LOG(DEBUG) << "Unbound texture";

  glBindFramebuffer(GL_FRAMEBUFFER, render_target_->framebuffer_handle());
  glBindRenderbuffer(GL_RENDERBUFFER, render_target_->renderbuffer_handle());
  LOG(DEBUG) << "Bound framebuffer " << render_target_->framebuffer_handle()
             << " and renderbuffer " << render_target_->renderbuffer_handle();
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB10_A2, render_target_->width(),
                        render_target_->height());

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         render_target_->texture_handle(), 0);

  LOG(DEBUG) << "Configured framebuffer texture as "
             << render_target_->texture_handle() << " for framebuffer "
             << render_target_->framebuffer_handle() << " and renderbuffer "
             << render_target_->renderbuffer_handle();
  GLenum draw_buffer = GL_COLOR_ATTACHMENT0;
  glDrawBuffers(1, &draw_buffer);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    LOG(ERROR) << "Failed to configure framebuffer";
    return;
  } else {
    LOG(DEBUG) << "Successfully configured framebuffer";
  }

  glViewport(0, 0, render_target_->width(), render_target_->height());
  LOG(DEBUG) << "Configured viewport: " << render_target_->width() << "x"
             << render_target->height();
}

GlRenderTargetActivation::~GlRenderTargetActivation() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  LOG(DEBUG) << "Unbound framebuffer";
}

GlRenderTarget::GlRenderTarget(int width, int height)
    : width_(width), height_(height) {
  // Create a new renderbuffer.
  glGenFramebuffers(1, &renderbuffer_handle_);
  LOG(DEBUG) << "Generated renderbuffer: " << renderbuffer_handle_;
  // Create a new framebuffer.
  glGenFramebuffers(1, &framebuffer_handle_);
  LOG(DEBUG) << "Generated framebuffer: " << framebuffer_handle_;
  // Generate a backing texture.
  glGenTextures(1, &texture_handle_);
  LOG(DEBUG) << "Generated texture: " << texture_handle_;
}

GlRenderTarget::~GlRenderTarget() {
  LOG(DEBUG) << "Disposing render target";
  glDeleteTextures(1, &texture_handle_);
  glDeleteFramebuffers(1, &framebuffer_handle_);
}

void GlRenderTarget::UpdateGeometry(int width, int height) {
  std::unique_lock<std::mutex> lock(render_target_mu_);
  width_ = width;
  height_ = height;
}

std::shared_ptr<GlRenderTargetActivation> GlRenderTarget::Activate() {
  return std::make_shared<GlRenderTargetActivation>(shared_from_this());
}

}  // namespace gl
