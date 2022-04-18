#include "util/graph/conversion.h"

#include <ostream>

#include "util/logging/logging.h"

namespace opendrop {

std::ostream& operator<<(std::ostream& os, const Conversion& conversion) {
  return os << "Conversion(input_types = " << conversion.input_types
            << ", output_types = " << conversion.output_types << ")";
}
std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<Conversion>& conversion) {
  if (conversion == nullptr) {
    LOG(INFO) << "<nullptr>";
    return os;
  }
  LOG(INFO) << "Formatting shared_ptr<Conversion>";
  return os << *conversion.get();
}

}  // namespace opendrop
