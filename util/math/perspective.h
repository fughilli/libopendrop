#ifndef UTIL_PERSPECTIVE_H_
#define UTIL_PERSPECTIVE_H_

#include "util/glm_helper.h"

namespace opendrop {

struct Directions {
  // TODO: Figure out why constexpr is not working with GLM on RPi.
  // Maybe something related to detection of supported compiler features in (?):
  // https://github.com/g-truc/glm/blob/b3f87720261d623986f164b2a7f6a0a938430271/glm/detail/setup.hpp#L280
  static const glm::vec3 kUp;
  static const glm::vec3 kDown;
  static const glm::vec3 kRight;
  static const glm::vec3 kLeft;
  static const glm::vec3 kIntoScreen;
  static const glm::vec3 kOutOfScreen;
};

glm::mat3x3 OrientTowards(glm::vec3 look_ray);

}  // namespace opendrop

#endif  // UTIL_PERSPECTIVE_H_
