#ifndef LIBOPENDROP_UTIL_MATH_H_
#define LIBOPENDROP_UTIL_MATH_H_

#include <cmath>

#include "util/logging.h"

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

template <typename T>
T LogLinear(T arg, T n) {
  // TODO: This implementation should find some more interesting analytic
  // interpolation between linear and logarithmic such that the curve does not
  // asymptotically approach the log regime as arg=>0, and the linear regime as
  // arg=>inf.
  //
  // Perhaps an answer lies at
  // <https://math.stackexchange.com/questions/107245/continuum-between-linear-and-logarithmic>?
  //
  // The goal is to be able to undo exponentiation on a value so that it can be
  // mapped to a range between 0 and 1.
  CHECK(arg > 0) << "arg must be positive.";
  CHECK(n <= 1 && n >= 0) << "n must be between 0 and 1, inclusive.";
  return n * arg + (1 - n) * std::log(arg);
}

template <typename T, bool clamp = false>
T MapValue(T arg, T in_low, T in_high, T out_low, T out_high) {
  if constexpr (clamp) {
    if (arg < in_low) return out_low;
    if (arg > in_high) return out_high;
  }
  return (arg - in_low) / (in_high - in_low) * (out_high - out_low) + out_low;
}

}  // namespace opendrop

#endif  // LIBOPENDROP_UTIL_MATH_H_
