#ifndef PRIMITIVE_POLYLINE_H_
#define PRIMITIVE_POLYLINE_H_

#include <cstdint>
#include <vector>

#include "absl/types/span.h"
#include "primitive/primitive.h"
#include "third_party/glm_helper.h"

namespace opendrop {

// Polyline takes a span of vertices. It is assumed that the lifetime of the
// underlying data exceeds that of the Polyline instance.
class Polyline : public Primitive {
 public:
  Polyline();
  Polyline(glm::vec3 color, absl::Span<const glm::vec2> vertices, float width);
  void Draw() override;

  // Updates the const view of vertices for this polyline.
  void UpdateVertices(absl::Span<const glm::vec2> vertices);

  // Updates the color of the polyline.
  void UpdateColor(glm::vec3 color);

  // Updates the width of this polyline.
  void UpdateWidth(float width);

 private:
  glm::vec3 color_;
  absl::Span<const glm::vec2> vertices_;
  float width_;
  std::vector<uint16_t> indices_;
};

}  // namespace opendrop

#endif  // PRIMITIVE_POLYLINE_H_
