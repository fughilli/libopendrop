#ifndef UTIL_TESTING_GRAPH_MATCHERS_H_
#define UTIL_TESTING_GRAPH_MATCHERS_H_

#include "util/graph/graph.h"
#include "util/math/math.h"

namespace opendrop {
namespace graph_testing {

template <typename V>
std::string GraphPrintToString(const V& v) {
  std::stringstream ss;
  ss << v;
  return ss.str();
}

MATCHER_P2(TextureIsNear, other, epsilon, "") {
  const auto difference = arg - other;
  const float distance = difference.Length();

  if (distance < kEpsilon) return true;

  *result_listener << absl::StrFormat(
      "\n%s is within %f of\n%s (%f !< %f)\n(a - b) =\n%s",
      GraphPrintToString(arg), epsilon, GraphPrintToString(other), distance,
      epsilon, GraphPrintToString(difference));
  return false;
}

}  // namespace graph_testing
}  // namespace opendrop

#endif  // UTIL_TESTING_GRAPH_MATCHERS_H_
