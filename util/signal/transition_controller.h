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
    float threshold = 0.8f;
    float closeness_threshold = 1e-3f;
  };

  TransitionController(Options options)
      : options_(std::move(options)),
        decay_towards_{std::clamp(options_.decay_rate, 0.0f, 1.0f)} {}

  void Update(float input) {
    transitioned_last_update_ = false;
    switch (state_) {
      case kUserControlled: {
        input = std::clamp(input, 0.0f, 1.0f);
        decay_towards_.value() += input * options_.input_scale;

        if (input < options_.input_decay_zone)
          decay_towards_.Decay(
              0, /*strength=*/(1.0f - input / options_.input_decay_zone));

        if (decay_towards_.value() > options_.threshold) state_ = kLeadingOut;
        break;
      }
      case kLeadingOut: {
        decay_towards_.Decay(1.0f);
        if ((1.0f - decay_towards_.value()) < options_.closeness_threshold) {
          ++count_;
          transitioned_last_update_ = true;
          state_ = kLeadingIn;
          decay_towards_.value() = 0;
        }
        break;
      }
      case kLeadingIn: {
        decay_towards_.Decay(1.0f);
        if ((1.0f - decay_towards_.value()) < options_.closeness_threshold) {
          state_ = kUserControlled;
          decay_towards_.value() = 0;
        }
        break;
      }
    }
  }

  float value() { return decay_towards_.value(); }

  float LeadInValue() {
    if (state_ != kLeadingIn) return 1.0f;
    return value();
  }
  float LeadOutValue() {
    if (state_ == kLeadingIn) return 0.0f;
    return value();
  }

  int TransitionCount() { return count_; }

  bool Transitioned() { return transitioned_last_update_; }

 private:
  enum State {
    kLeadingIn,
    kUserControlled,
    kLeadingOut,
  } state_ = kUserControlled;

  Options options_;
  int count_ = 0;
  bool transitioned_last_update_ = false;
  DecayTowards decay_towards_;
};
}  // namespace opendrop

#endif  // UTIL_SIGNAL_TRANSITION_CONTROLLER_H_
