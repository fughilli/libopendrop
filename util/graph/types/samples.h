#ifndef UTIL_GRAPH_TYPES_SAMPLES_H_
#define UTIL_GRAPH_TYPES_SAMPLES_H_

#include "util/graph/types/opaque_storable.h"
#include "util/graph/types/types.h"

namespace opendrop {

struct Samples : public OpaqueStorable<Samples> {
  constexpr static Type kType = Type::kSamples;

  absl::Span<const float> samples_left;
  absl::Span<const float> samples_right;
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_SAMPLES_H_
