#ifndef UTIL_GRAPH_TYPES_TEXTURE_H_
#define UTIL_GRAPH_TYPES_TEXTURE_H_

#include <stddef.h>

#include <ostream>

#include "third_party/glm_helper.h"
#include "util/graph/types/types.h"
#include "util/logging/logging.h"

namespace opendrop {

class Texture {
 public:
  constexpr static Type kType = Type::kTexture;

  Texture() : width_(0), height_(0) {}
  Texture(size_t width, size_t height) : width_(width), height_(height) {}

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

  static Texture SolidColor(glm::vec4 color, size_t width, size_t height) {
    // std::vector<glm::vec4> solid(width * height * 4, 0);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
    // GL_UNSIGNED_BYTE, &emptyData[0]);
    Texture tex(width, height);
    tex.color_ = color;
    return tex;
  }

  glm::vec4 Color() const { return color_; }

 private:
  glm::vec4 color_ = {};
  size_t width_, height_;
};

std::ostream& operator<<(std::ostream& os, const Texture& texture);

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_TEXTURE_H_
