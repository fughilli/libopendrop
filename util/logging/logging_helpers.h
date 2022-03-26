#ifndef UTIL_LOGGING_LOGGING_HELPERS_H_
#define UTIL_LOGGING_LOGGING_HELPERS_H_

#include <iostream>

#include "absl/types/span.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, const absl::Span<T>& span) {
  os << "[";
  for (int i = 0; i < span.size(); ++i) {
    os << span[i];
    if (i != span.size() - 1) {
      os << ", ";
    }
  }
  os << "]";
  return os;
}

#endif  // UTIL_LOGGING_LOGGING_HELPERS_H_
