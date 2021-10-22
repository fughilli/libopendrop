#ifndef UTIL_COEFFICIENTS_H_
#define UTIL_COEFFICIENTS_H_

#include <array>
#include <random>

#include "libopendrop/util/logging.h"

namespace opendrop {

class Coefficients {
 public:
  // Returns random coefficients distributed in the given range.
  template <int N>
  static std::array<float, N> Random(float minimum, float maximum) {
    static std::random_device device;
    static std::default_random_engine random_engine(device());
    CHECK(minimum <= maximum)
        << "minimum must be less than or equal to maximum";
    std::uniform_real_distribution<float> distribution(minimum, maximum);

    std::array<float, N> return_coefficients;
    for (int i = 0; i < N; ++i) {
      return_coefficients[i] = distribution(random_engine);
    }

    return return_coefficients;
  }
};

}  // namespace opendrop

#endif  // UTIL_COEFFICIENTS_H_
