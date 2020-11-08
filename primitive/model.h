#ifndef PRIMITIVES_MODEL_H_
#define PRIMITIVES_MODEL_H_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "libopendrop/primitive/primitive.h"

namespace opendrop {

class Model : public Primitive {
 public:
  Model(const glm::vec3* vertices, const glm::vec2* uvs,
        const glm::ivec3* triangles)
      : vertices_(vertices), uvs_(uvs), triangles_(triangles) {}
  void Draw() override;

 private:
  const glm::vec3* vertices_;
  const glm::vec2* uvs_;
  const glm::ivec3* triangles_;
};

}  // namespace opendrop

#endif  // PRIMITIVES_MODEL_H_
