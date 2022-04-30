#include "util/math/math.h"

#include <cmath>

namespace opendrop {

float UnitarySin(float x) { return (1.0f + std::sin(x)) / 2; }

}  // namespace opendrop
