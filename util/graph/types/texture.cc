#include "util/graph/types/texture.h"

#include <ostream>

#include "util/logging/logging.h"
#include "absl/strings/str_format.h"

namespace opendrop {

std::ostream& operator<<(std::ostream& os, const Texture& texture) {
  return os << absl::StrFormat("Texture(%s)", ToString(texture.Color()));
}

}  // namespace opendrop
