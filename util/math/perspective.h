#include <glm/glm.hpp>

namespace opendrop {

struct Directions {
  static constexpr glm::vec3 kUp = {0, 1, 0};
  static constexpr glm::vec3 kDown = {0, -1, 0};
  static constexpr glm::vec3 kRight = {1, 0, 0};
  static constexpr glm::vec3 kLeft = {-1, 0, 0};
  static constexpr glm::vec3 kIntoScreen = {0, 0, 1};
  static constexpr glm::vec3 kOutOfScreen = {0, 0, -1};
};

glm::mat3x3 OrientTowards(glm::vec3 look_ray);

}  // namespace opendrop
