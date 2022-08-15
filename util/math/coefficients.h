#ifndef UTIL_MATH_COEFFICIENTS_H_
#define UTIL_MATH_COEFFICIENTS_H_

#include <array>
#include <random>
#include <type_traits>
#include <chrono>

#include "util/logging/logging.h"

namespace opendrop {

class Coefficients {
 public:
  // Returns random coefficients distributed in the given range.
  template <int N, typename T = float,
            std::enable_if_t<std::is_floating_point<T>::value, void*> = nullptr>
  static std::array<T, N> Random(T minimum, T maximum) {
    static std::random_device device;
    static std::default_random_engine random_engine(device());
    CHECK(minimum <= maximum)
        << "minimum must be less than or equal to maximum";
    std::uniform_real_distribution<T> distribution(minimum, maximum);

    std::array<T, N> return_coefficients;
    for (int i = 0; i < N; ++i) {
      return_coefficients[i] = distribution(random_engine);
    }

    return return_coefficients;
  }

  // Returns random coefficients distributed in the given range.
  template <int N, typename T = int,
            std::enable_if_t<std::is_integral<T>::value, void*> = nullptr>
  static std::array<T, N> Random(T minimum, T maximum) {
    static std::random_device device;
    static std::default_random_engine random_engine(device());
    CHECK(minimum <= maximum)
        << "minimum must be less than or equal to maximum";
    std::uniform_int_distribution<T> distribution(minimum, maximum);

    std::array<T, N> return_coefficients;
    for (int i = 0; i < N; ++i) {
      return_coefficients[i] = distribution(random_engine);
    }

    return return_coefficients;
  }
};

}  // namespace opendrop

#endif  // UTIL_MATH_COEFFICIENTS_H_
