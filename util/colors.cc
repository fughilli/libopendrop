#include "libopendrop/util/colors.h"

namespace opendrop {

glm::vec3 HsvToRgb(glm::vec3 hsv) {
  auto normalized_offset_sin = [&](float x, float offset) {
    return (1.0f + sin((x + offset) * M_PI * 2)) / 2;
  };
  return glm::vec3(normalized_offset_sin(hsv.x, 0.0f),
                   normalized_offset_sin(hsv.x, 0.333f),
                   normalized_offset_sin(hsv.x, 0.666f));
}

}  // namespace opendrop
