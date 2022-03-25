#ifndef SDL_GL_INTERFACE_H_
#define SDL_GL_INTERFACE_H_

#include <SDL2/SDL.h>

#include <memory>

#include "gl_interface.h"

namespace gl {

struct SdlWindowDestroyer {
  void operator()(SDL_Window* window) {
    if (window == nullptr) {
      return;
    }
    SDL_DestroyWindow(window);
  }
};

class SdlGlInterface;

// RAII wrapper for an activated GLContext. The GLContext is made active on
// construction, and made inactive on destruction.
class SdlGlContextActivation : public GlContextActivation {
 public:
  SdlGlContextActivation(std::shared_ptr<SdlGlInterface> interface,
                         SDL_GLContext context);
  virtual ~SdlGlContextActivation();

 private:
  std::shared_ptr<SdlGlInterface> interface_;
  SDL_GLContext context_;
};

// Represents a GLContext.
class SdlGlContext : public GlContext {
 public:
  SdlGlContext(std::shared_ptr<SdlGlInterface> interface,
               SDL_GLContext context);
  virtual ~SdlGlContext();

  std::shared_ptr<GlContextActivation> Activate() override;

  SDL_GLContext* get() { return &context_; }

 private:
  std::shared_ptr<SdlGlInterface> interface_;
  SDL_GLContext context_;
};

// Represents a GL API interface.
class SdlGlInterface : public GlInterface,
                       public std::enable_shared_from_this<SdlGlInterface> {
 public:
  // Constructs an SdlGlInterface with the given window. The window is owned by
  // this SdlGlInterface after construction.
  SdlGlInterface(SDL_Window* window);
  virtual ~SdlGlInterface() {}

  // Allocates a new shared GLContext.
  std::shared_ptr<GlContext> AllocateSharedContext() override;

  void SetVsync(bool enable) override;

  glm::ivec2 DrawableSize() override;

  // Swaps the window double buffers for the window associated with this
  // SdlGlInterface.
  void SwapBuffers() override;

  std::shared_ptr<SDL_Window> GetWindow() { return window_; }

 private:
  // Window managed by this SdlGlInterface.
  std::shared_ptr<SDL_Window> window_;
};

}  // namespace gl

#endif  // SDL_GL_INTERFACE_H_
