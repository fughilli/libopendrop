#include <cmath>
#include <glm/glm.hpp>

namespace opendrop {

glm::vec2 UnitVectorAtAngle(float angle) {
  return glm::vec2(std::cos(angle), std::sin(angle));
}

glm::vec2 Rotate2d(glm::vec2 vector, float angle) {
  float cos_angle = std::cos(angle);
  float sin_angle = std::sin(angle);

  return glm::vec2(vector.x * cos_angle - vector.y * sin_angle,
                   vector.x * sin_angle + vector.y * cos_angle);
}

}  // namespace opendrop
