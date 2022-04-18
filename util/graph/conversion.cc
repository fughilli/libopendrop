#include "util/graph/conversion.h"

#include <ostream>

#include "absl/strings/str_format.h"
#include "util/logging/logging.h"

namespace opendrop {

std::ostream& operator<<(std::ostream& os, const Conversion& conversion) {
  return os << absl::StrFormat(
             "[%s -> %s]",
             ToString(conversion.input_types), ToString(conversion.output_types));
}
std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<Conversion>& conversion) {
  if (conversion == nullptr) {
    return os << "(Conversion*)(nullptr)";
  }
  return os << *conversion.get();
}

}  // namespace opendrop
