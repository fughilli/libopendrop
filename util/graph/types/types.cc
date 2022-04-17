#include "util/graph/types/types.h"

#include <list>
#include <ostream>
#include <vector>

namespace opendrop {

std::ostream& operator<<(std::ostream& os, Type type) {
#define CASE(x)    \
  case Type::k##x: \
    os << "k" #x;  \
    break;
  switch (type) {
    CASE(FloatGeneric);
    CASE(Unitary);
    CASE(Monotonic);
    CASE(Texture);
    CASE(Integer);
  }
#undef CASE
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Type>& types) {
  for (int i = 0; i < types.size(); ++i) {
    os << types[i];
    if (i != types.size() - 1) os << ", ";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::list<Type>& types) {
  for (auto iter = types.begin(); iter != types.end(); ++iter) {
    os << *iter << ", ";
  }
  return os;
}

template <>
Type ToType<int>() {
  return Type::kInteger;
}
template <>
Type ToType<float>() {
  return Type::kFloatGeneric;
}

std::vector<Type> ConstructTypes() { return {}; }
std::vector<Type> ConstructTypesFromTuple(const std::tuple<>&) { return {}; }

}  // namespace opendrop
