#include "absl/strings/str_format.h"
#include "googlemock/include/gmock/gmock-matchers.h"
#include "re2/re2.h"

namespace opendrop {
namespace re2_testing {

MATCHER_P(MatchesRegex, pattern, "") {
  if (RE2 ::FullMatch(arg, RE2(pattern))) return true;

  *result_listener << absl::StrFormat("%s matches regex `%s`", arg, pattern);
  return false;
}

}  // namespace re2_testing
}  // namespace opendrop
