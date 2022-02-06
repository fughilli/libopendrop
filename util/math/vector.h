#ifndef UTIL_MATH_VECTOR_H_
#define UTIL_MATH_VECTOR_H_

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

namespace opendrop {

glm::vec2 UnitVectorAtAngle(float angle);

// Rotates a vector counterclockwise by an angle in radians.
glm::vec2 Rotate2d(glm::vec2 vector, float angle);

// Generates a transform that takes an object to a position along a ring of
// radius `radius`. The position along the ring is given in turns; when
// `position_along_ring` is 0, the object is positioned at <radius, 0>. The
// object moves counterclockwise with increasing `position_along_ring`.
glm::mat4 RingTransform(glm::vec3 normal, float radius,
                        float position_along_ring);

// Generates a transform that takes an object to a position
glm::mat4 LineTransform(glm::vec3 start_position, glm::vec3 end_position,
                        float position_along_line);

}  // namespace opendrop

#endif  // UTIL_MATH_VECTOR_H_
