#ifndef PRIMITIVE_MODEL_H_
#define PRIMITIVE_MODEL_H_

#include "absl/types/span.h"
#include "primitive/primitive.h"
#include "util/glm_helper.h"

namespace opendrop {

class Model : public Primitive {
 public:
  Model(absl::Span<const glm::vec3> vertices, absl::Span<const glm::vec2> uvs,
        absl::Span<const glm::uvec3> triangles)
      : vertices_(vertices), normals_({}), uvs_(uvs), triangles_(triangles) {}
  Model(absl::Span<const glm::vec3> vertices,
        absl::Span<const glm::vec3> normals, absl::Span<const glm::vec2> uvs,
        absl::Span<const glm::uvec3> triangles)
      : vertices_(vertices),
        normals_(normals),
        uvs_(uvs),
        triangles_(triangles) {}
  void Draw() override;

 private:
  absl::Span<const glm::vec3> vertices_;
  absl::Span<const glm::vec3> normals_;
  absl::Span<const glm::vec2> uvs_;
  absl::Span<const glm::uvec3> triangles_;
};

}  // namespace opendrop

#endif  // PRIMITIVE_MODEL_H_
