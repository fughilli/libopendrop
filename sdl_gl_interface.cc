#include "libopendrop/sdl_gl_interface.h"

#include <iostream>

namespace gl {

// SdlGlContextActivation implementation
// ============================================================================
SdlGlContextActivation::SdlGlContextActivation(
    std::shared_ptr<SdlGlInterface> interface, SDL_GLContext context)
    : interface_(interface), context_(context) {
  // Activate the GL context.
  SDL_GL_MakeCurrent(interface_->GetWindow().get(), context_);
}

SdlGlContextActivation::~SdlGlContextActivation() {
  // Deactivate the GL context.
  SDL_GL_MakeCurrent(interface_->GetWindow().get(), 0);
}

// SdlGlContext implementation
// ============================================================================
SdlGlContext::SdlGlContext(std::shared_ptr<SdlGlInterface> interface,
                           SDL_GLContext context)
    : interface_(interface), context_(context) {}

SdlGlContext::~SdlGlContext() { SDL_GL_DeleteContext(context_); }

std::shared_ptr<GlContextActivation> SdlGlContext::Activate() {
  return std::make_shared<SdlGlContextActivation>(interface_, context_);
}

// SdlGlInterface implementation
// ============================================================================
SdlGlInterface::SdlGlInterface(SDL_Window* window)
    : window_(window, SdlWindowDestroyer()) {
  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
}

std::shared_ptr<GlContext> SdlGlInterface::AllocateSharedContext() {
  auto sdl_gl_context = std::make_shared<SdlGlContext>(
      shared_from_this(), SDL_GL_CreateContext(window_.get()));
  return sdl_gl_context;
}

void SdlGlInterface::SetVsync(bool enable) {
  if (enable) {
    int avsync = SDL_GL_SetSwapInterval(-1);
    if (avsync == -1) {
      SDL_GL_SetSwapInterval(1);
    }
    return;
  }

  SDL_GL_SetSwapInterval(0);
}

void SdlGlInterface::SwapBuffers() { SDL_GL_SwapWindow(window_.get()); }

}  // namespace gl
