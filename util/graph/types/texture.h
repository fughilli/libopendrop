#ifndef UTIL_GRAPH_TYPES_TEXTURE_H_
#define UTIL_GRAPH_TYPES_TEXTURE_H_

#include <stddef.h>

#include <ostream>

#include "util/graph/types/opaque_storable.h"
#include "absl/strings/str_format.h"
#include "third_party/glm_helper.h"
#include "util/graph/types/types.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "util/logging/logging.h"

namespace opendrop {

class Texture : public OpaqueStorable<Texture> {
 public:
  constexpr static Type kType = Type::kTexture;

  Texture() : width_(0), height_(0) {}
  Texture(size_t width, size_t height) : width_(width), height_(height) {}
  Texture(size_t width, size_t height,
          std::shared_ptr<gl::GlTextureManager> texture_manager)
      : width_(width), height_(height) {
    {
      auto status_or_render_target =
          gl::GlRenderTarget::MakeShared(width_, height_, texture_manager);
      if (!status_or_render_target.ok()) {
        LOG(ERROR) << "Texture::Texture(): Render target creation failed: "
                   << status_or_render_target.status();
        return;
      }
      LOG(INFO)
          << "Texture::Texture(): status-embedded render target has use count "
          << status_or_render_target.value().use_count();
      render_target_ = status_or_render_target.value();
      LOG(INFO) << "Texture::Texture(): post-assignment use count is "
                << render_target_.use_count();
    }
    LOG(INFO) << "Texture::Texture(): post-descoping use count is "
              << render_target_.use_count();
  }

  Texture operator-(const Texture& other) const {
    if (width_ != other.width_ || height_ != other.height_)
      LOG(FATAL) << absl::StrFormat(
          "Width and height don't match (lhs = %dx%d, rhs = %dx%d)", width_,
          height_, other.width_, other.height_);

    Texture ret(width_, height_);
    ret.color_ = color_ - other.color_;
    return ret;
  }
  // Compute the geometric mean of the amplitude of the raster.
  float Length() const { return glm::length(color_); }

  // static Texture Allocate(gl::GlTextureManager& texture_manager) {
  //   absl::StatusOr<int> texture_unit = texture_manager.Allocate();
  //   CHECK_OK(texture_unit);
  //   return Texture { .texture_unit = texture_unit.get(); };
  // }
  //
  int ActivateRenderContext() { return 0; }

  std::shared_ptr<gl::GlRenderTarget>& RenderTarget() { return render_target_; }
  std::shared_ptr<gl::GlRenderTarget> RenderTarget() const {
    return render_target_;
  }

  static Texture SolidColor(glm::vec4 color, size_t width, size_t height) {
    // std::vector<glm::vec4> solid(width * height * 4, 0);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
    // GL_UNSIGNED_BYTE, &emptyData[0]);
    Texture tex(width, height);
    tex.color_ = color;
    return tex;
  }

  glm::vec4 Color() const { return color_; }

  ~Texture() {
    LOG(INFO)
        << "Destructing graph Texture; render_target_ "
        << ((render_target_ == nullptr)
                ? "is nullptr"
                : absl::StrFormat(
                      "has use count %d and holds {framebuffer_handle_ = %d, "
                      "depth_buffer_handle_ = %d}",
                      render_target_.use_count(),
                      render_target_->framebuffer_handle(),
                      render_target_->depth_buffer_handle()));
  }

 private:
  glm::vec4 color_ = {};
  size_t width_, height_;
  std::shared_ptr<gl::GlRenderTarget> render_target_ = nullptr;
};

void Blit(const Texture& texture);

std::ostream& operator<<(std::ostream& os, const Texture& texture);

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_TEXTURE_H_
