#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>

#include "util/logging.h"
#include "util/math/constants.h"

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

glm::mat4 RingTransform(glm::vec3 normal, float radius,
                        float position_along_ring) {
  glm::mat4 translation = Constants::kI4x4;
  translation[3] = glm::vec4(radius, 0, 0, 1);
  return glm::rotate(Constants::kI4x4, position_along_ring, normal) *
         translation;
}

}  // namespace opendrop
