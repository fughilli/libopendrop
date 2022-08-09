#include "util/logging/logging_glm_helpers.h"

#include <iomanip>
#include <sstream>

#include "absl/strings/str_format.h"

namespace glm {

std::ostream& operator<<(std::ostream& os, const vec2& vec) {
  os << "<" << vec.x << ", " << vec.y << ">";
  return os;
}

std::ostream& operator<<(std::ostream& os, const vec3& vec) {
  os << "<" << vec.x << ", " << vec.y << ", " << vec.z << ">";
  return os;
}

std::ostream& operator<<(std::ostream& os, const vec4& vec) {
  os << "<" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ">";
  return os;
}

std::ostream& operator<<(std::ostream& os, const mat4& vec) {
  std::stringstream ss;

  constexpr char kFormatStr[] = "| % 10.4f % 10.4f % 10.4f % 10.4f |\n";

  for (int i = 0; i < 4; ++i)
    ss << absl::StrFormat(kFormatStr, vec[0][i], vec[1][i], vec[2][i],
                          vec[3][i]);
  return os << ss.str();
}

}  // namespace glm
