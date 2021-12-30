#include <glm/glm.hpp>

namespace opendrop {

glm::vec2 UnitVectorAtAngle(float angle);

// Rotates a vector counterclockwise by an angle in radians.
glm::vec2 Rotate2d(glm::vec2 vector, float angle);

}  // namespace opendrop
