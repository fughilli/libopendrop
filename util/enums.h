#ifndef UTIL_ENUMS_H_
#define UTIL_ENUMS_H_

#include <cmath>

template <typename E>
E InterpolateEnum(float x) {
  return static_cast<E>(
      std::clamp<int>(static_cast<int>(E::kDenseLastValue + 1) * x, 0,
                      static_cast<int>(E::kDenseLastValue)));
}

#endif  // UTIL_ENUMS_H_
