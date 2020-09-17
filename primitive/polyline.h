#ifndef PRIMITIVES_POLYLINE_H_
#define PRIMITIVES_POLYLINE_H_

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "absl/types/span.h"
#include "libopendrop/primitive/primitive.h"

namespace opendrop {

// Polyline takes a span of vertices. It is assumed that the lifetime of the
// underlying data exceeds that of the Polyline instance.
class Polyline : public Primitive {
 public:
  Polyline(glm::vec3 color, absl::Span<const glm::vec2> vertices, float width);
  void Draw() override;

 private:
  glm::vec3 color_;
  absl::Span<const glm::vec2> vertices_;
  float width_;
  std::vector<uint16_t> indices_;
};

}  // namespace opendrop

#endif  // PRIMITIVES_POLYLINE_H_
