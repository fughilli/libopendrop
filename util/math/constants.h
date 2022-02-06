#ifndef UTIL_MATH_CONSTANTS_H_
#define UTIL_MATH_CONSTANTS_H_

#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

namespace opendrop {

struct Constants {
  static const glm::mat2x2 kI2x2;
  static const glm::mat3x3 kI3x3;
  static const glm::mat4x4 kI4x4;
};

}  // namespace opendrop

#endif  // UTIL_MATH_CONSTANTS_H_
