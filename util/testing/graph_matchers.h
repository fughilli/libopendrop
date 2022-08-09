#ifndef UTIL_TESTING_GRAPH_MATCHERS_H_
#define UTIL_TESTING_GRAPH_MATCHERS_H_

#include <ostream>

#include "absl/strings/str_format.h"
#include "googlemock/include/gmock/gmock-matchers.h"
#include "util/graph/graph.h"
#include "util/graph/types/color.h"
#include "util/logging/logging.h"

namespace opendrop {
namespace graph_testing {

MATCHER_P2(TextureIsNear, other, epsilon, "") {
  const auto difference = arg - other;
  const float distance = difference.Length();

  if (distance < epsilon) return true;

  *result_listener << absl::StrFormat(
      "\n%s is within %f of\n%s (%f !< %f)\n(a - b) =\n%s", ToString(arg),
      epsilon, ToString(other), distance, epsilon, ToString(difference));
  return false;
}

MATCHER_P2(ColorIsNear, other, epsilon, "") {
  const auto difference = arg - other;
  const float distance = difference.Length();

  if (distance < epsilon) return true;

  *result_listener << absl::StrFormat(
      "\n%s is within %f of\n%s (%f !< %f)\n(a - b) =\n%s", ToString(arg),
      epsilon, ToString(other), distance, epsilon, ToString(difference));
  return false;
}

}  // namespace graph_testing
}  // namespace opendrop

#endif  // UTIL_TESTING_GRAPH_MATCHERS_H_
