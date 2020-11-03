#include "libopendrop/util/colors.h"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>

namespace opendrop {

glm::vec3 HsvToRgb(glm::vec3 hsv) {
  // C = V x Sv
  float c = std::clamp(hsv.y, 0.0f, 1.0f) * std::clamp(hsv.z, 0.0f, 1.0f);

  // H' = H/60deg
  // X = chroma x (1 - |H' mod 2 - 1|)
  float x = c * (1.0f - std::abs(std::fmod(hsv.x * 6.0f, 2.0f) - 1.0f));

  switch (static_cast<int64_t>(std::floor(hsv.x * 6)) % 6) {
    case 0:
      return glm::vec3(c, x, 0.0f);
    case 1:
      return glm::vec3(x, c, 0.0f);
    case 2:
      return glm::vec3(0.0f, c, x);
    case 3:
      return glm::vec3(0.0f, x, c);
    case 4:
      return glm::vec3(x, 0.0f, c);
    case 5:
      return glm::vec3(c, 0.0f, x);
    default:
      return glm::vec3(0.0f, 0.0f, 0.0f);
  }
}

}  // namespace opendrop
