#include "util/math/perspective.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

namespace opendrop {

const glm::vec3 Directions::kUp = {0, 1, 0};
const glm::vec3 Directions::kDown = {0, -1, 0};
const glm::vec3 Directions::kRight = {1, 0, 0};
const glm::vec3 Directions::kLeft = {-1, 0, 0};
const glm::vec3 Directions::kIntoScreen = {0, 0, 1};
const glm::vec3 Directions::kOutOfScreen = {0, 0, -1};

glm::mat3x3 OrientTowards(glm::vec3 look_ray) {
  return glm::mat3x3(glm::lookAt(look_ray, glm::vec3(0), Directions::kUp));
}

}  // namespace opendrop
