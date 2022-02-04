#include "util/logging_glm_helpers.h"
#include <sstream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const glm::vec2& vec) {
  os << "<" << vec.x << ", " << vec.y << ">";
  return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
  os << "<" << vec.x << ", " << vec.y << ", " << vec.z << ">";
  return os;
}

std::ostream& operator<<(std::ostream& os, const glm::vec4& vec) {
  os << "<" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ">";
  return os;
}

std::ostream& operator<<(std::ostream& os, const glm::mat4& vec) {
  std::stringstream ss;

  ss << std::setw(4) << "|" << vec[0].x << " " << vec[1].x << " " << vec[2].x << " " << vec[3].x << "|" << std::endl;
  ss << std::setw(4) << "|" << vec[0].y << " " << vec[1].y << " " << vec[2].y << " " << vec[3].y << "|" << std::endl;
  ss << std::setw(4) << "|" << vec[0].z << " " << vec[1].z << " " << vec[2].z << " " << vec[3].z << "|" << std::endl;
  ss << std::setw(4) << "|" << vec[0].w << " " << vec[1].w << " " << vec[2].w << " " << vec[3].w << "|" << std::endl;
  return os << ss.str();
}
