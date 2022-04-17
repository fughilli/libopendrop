#ifndef UTIL_GRAPH_TYPES_MONOTONIC_H_
#define UTIL_GRAPH_TYPES_MONOTONIC_H_

#include "util/graph/types/types.h"

namespace opendrop {

struct Monotonic {
  constexpr static Type kType = Type::kMonotonic;

  float value;

  Monotonic() : value(0) {}
  Monotonic(float value) : value(value) {}
  operator float() const { return value; }
};

}

#endif // UTIL_GRAPH_TYPES_MONOTONIC_H_
