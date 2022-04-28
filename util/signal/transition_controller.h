#ifndef UTIL_SIGNAL_TRANSITION_CONTROLLER_H_
#define UTIL_SIGNAL_TRANSITION_CONTROLLER_H_

#include <algorithm>

#include "util/signal/decay_towards.h"

namespace opendrop {
class TransitionController {
 public:
  struct Options {
    float decay_rate = 0.1f;
    float input_decay_zone = 0.2f;
    float input_scale = 0.01f;
  };

  TransitionController(Options options)
      : options_(std::move(options)),
        decay_towards_{std::clamp(options_.decay_rate, 0.0f, 1.0f)} {}

  void Input(float input) {
    input = std::clamp(input, 0.0f, 1.0f);
    decay_towards_.value() += input * options_.input_scale;

    if (input < options_.input_decay_zone)
      decay_towards_.Decay(
          0, /*strength=*/(1.0f - input / options_.input_decay_zone));
  }

  float value() { return decay_towards_.value(); }

 private:
  Options options_;
  DecayTowards decay_towards_;
};
}  // namespace opendrop

#endif  // UTIL_SIGNAL_TRANSITION_CONTROLLER_H_
