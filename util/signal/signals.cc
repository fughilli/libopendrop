#include "util/signal/signals.h"

namespace opendrop {

float SineEase(float arg) {
  arg = std::clamp<float>(arg, 0.0f, 1.0f);
  return 1.0f - (cos(arg * M_PI) + 1.0f) / 2.0f;
}

}  // namespace opendrop
