#ifndef UTIL_GRAPH_TYPES_COLOR_H_
#define UTIL_GRAPH_TYPES_COLOR_H_

#include "third_party/glm_helper.h"
#include "util/graph/types/types.h"

namespace opendrop {

struct Color {
  constexpr static Type kType = Type::kColor;

  glm::vec4 value;

  Color() : value(glm::vec4(0, 0, 0, 1)) {}
  Color(glm::vec4 value) : value(value) {}
  operator glm::vec4() const { return value; }

  Color operator-(Color other) const { return value - other.value; }

  float Length() const { return glm::length(value); }
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_COLOR_H_
