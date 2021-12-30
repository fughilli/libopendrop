#include "util/math/perspective.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace opendrop {

glm::mat3x3 OrientTowards(glm::vec3 look_ray) {
  return glm::mat3x3(glm::lookAt(look_ray, glm::vec3(0), Directions::kUp));
}

}  // namespace opendrop
