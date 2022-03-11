#ifndef UTIL_ENUM_H_
#define UTIL_ENUM_H_

#include <cmath>

template <typename E>
E InterpolateEnum(float x) {
  return static_cast<E>(
      std::clamp<int>(static_cast<int>(E::kDenseLastValue + 1) * x, 0,
                      static_cast<int>(E::kDenseLastValue)));
}

#endif  // UTIL_ENUM_H_
