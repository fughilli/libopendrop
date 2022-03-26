#ifndef UTIL_TESTING_GLM_MATCHERS_H_
#define UTIL_TESTING_GLM_MATCHERS_H_

#include <ostream>
#include <sstream>
#include <string>

#include "absl/strings/str_format.h"
#include "googlemock/include/gmock/gmock-matchers.h"
#include "third_party/glm_helper.h"
#include "util/logging/logging_glm_helpers.h"

// See matcher authorship guide at
// https://github.com/google/googletest/blob/main/docs/reference/matchers.md#defining-matchers

namespace opendrop {
namespace glm_testing {

template <typename V>
float Length(const V& v) {
  return glm::length(v);
}
template <>
float Length<glm::mat4>(const glm::mat4& m) {
  float accum = 0.0f;
  for (int col = 0; col < 4; ++col)
    accum += (glm::length(m[col]) * glm::length(m[col]));
  return std::sqrt(accum);
}

template <typename V>
std::string GlmPrintToString(const V& v) {
  std::stringstream ss;
  ss << v;
  return ss.str();
}

MATCHER_P2(IsNear, other, epsilon, "") {
  const auto difference = arg - other;
  const float distance = Length(difference);

  if (distance < kEpsilon) return true;

  *result_listener << absl::StrFormat(
      "\n%s is within %f of\n%s (%f !< %f)\n(a - b) =\n%s",
      GlmPrintToString(arg), epsilon, GlmPrintToString(other), distance,
      epsilon, GlmPrintToString(difference));
  return false;
}

}  // namespace glm_testing
}  // namespace opendrop

#endif  // UTIL_TESTING_GLM_MATCHERS_H_
