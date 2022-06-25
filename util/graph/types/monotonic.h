#ifndef UTIL_GRAPH_TYPES_MONOTONIC_H_
#define UTIL_GRAPH_TYPES_MONOTONIC_H_

#include "util/graph/types/opaque_storable.h"
#include "util/graph/types/types.h"

namespace opendrop {

struct Monotonic : public OpaqueStorable<Monotonic> {
  constexpr static Type kType = Type::kMonotonic;

  float value;

  Monotonic() : value(0) {}
  Monotonic(float value) : value(value) {}
  operator float() const { return value; }
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_MONOTONIC_H_
