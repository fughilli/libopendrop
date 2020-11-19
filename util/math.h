#ifndef LIBOPENDROP_UTIL_MATH_H_
#define LIBOPENDROP_UTIL_MATH_H_

#include <cmath>

namespace opendrop {

constexpr float kEpsilon = 1e-6f;

template <typename T>
inline bool AlmostEqual(T a, T b, T bound = kEpsilon) {
  return std::abs(a - b) <= bound;
}

template <typename T>
inline T SafeDivide(T a, T b) {
  if (AlmostEqual<T>(b, 0)) {
    return a;
  }
  return a / b;
}

}  // namespace opendrop

#endif  // LIBOPENDROP_UTIL_MATH_H_
