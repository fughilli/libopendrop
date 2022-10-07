#include "util/graph/types/color.h"

namespace opendrop {
std::ostream& operator<<(std::ostream& os, const opendrop::Color& color) {
  os << absl::StrFormat("Color(r = %0.4f, g = %0.4f, b = %0.4f, a = %0.4f)",
                        color.value.r, color.value.g, color.value.b,
                        color.value.a);
  return os;
}
}  // namespace opendrop
