#include "util/audio/beat_estimator.h"

namespace opendrop {

std::ostream& operator<<(std::ostream& os, const std::vector<int>& ints) {
  for (size_t i = 0; i < ints.size(); ++i) {
    os << ints[i];
    if (i < ints.size() - 1) os << ",";
  }
  return os;
}

}  // namespace opendrop
