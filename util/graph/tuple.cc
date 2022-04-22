#include "util/graph/tuple.h"

#include <ostream>

namespace opendrop {

std::ostream& operator<<(std::ostream& os, const OpaqueTuple& tuple) {
  return os << "OpaqueTuple(.types = " << tuple.Types() << ")";
}

}  // namespace opendrop
