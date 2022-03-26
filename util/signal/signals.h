#ifndef UTIL_SIGNALS_H_
#define UTIL_SIGNALS_H_

#include <algorithm>
#include <cmath>

#include "util/math/coefficients.h"
#include "util/math/math.h"
#include "util/time/oneshot.h"

namespace opendrop {

class RandomEvent {
 public:
  RandomEvent(float min_period, float max_period)
      : min_period_(min_period), max_period_(max_period), oneshot_(0) {
    oneshot_.Start(0);
  }

  bool IsDue(float current_time) {
    if (oneshot_.IsDue(current_time)) {
      oneshot_ = Oneshot{Coefficients::Random<1>(min_period_, max_period_)[0]};
      oneshot_.Start(current_time);

      if (!initialized_) {
        initialized_ = true;
        return false;
      }

      return true;
    }
    return false;
  }

  // For debugging.
  Oneshot<float>& oneshot() { return oneshot_; }

 private:
  float min_period_, max_period_;
  Oneshot<float> oneshot_;
  bool initialized_ = false;
};

// Produces a sinusoidal easing from 0 to 1 for argument values between 0 and 1.
// The input is clamped to the valid range.
float SineEase(float arg);

class RampTweener {
 public:
  struct Options {
    float ramp_on_length;
    float on_length;
    float ramp_off_length;
  };

  RampTweener(Options options) : options_(std::move(options)) {}

  void Start(float start_time) {
    start_time_ = start_time;
    on_start_time_ = start_time_ + options_.ramp_on_length;
    off_start_time_ = on_start_time_ + options_.on_length;
    off_time_ = off_start_time_ + options_.ramp_off_length;
  }

  float Value(float current_time) {
    if (current_time < start_time_) return 0.0f;
    if (current_time > off_time_) return 0.0f;
    if (current_time <= on_start_time_)
      return MapValue<float>(current_time, start_time_, on_start_time_, 0, 1);
    if (current_time <= off_start_time_) return 1.0f;
    if (current_time <= off_time_)
      return MapValue<float>(current_time, off_start_time_, off_time_, 1, 0);
    return 0.0f;
  }

 private:
  Options options_;
  float start_time_;

  float on_start_time_;
  float off_start_time_;
  float off_time_;
};

}  // namespace opendrop

#endif  // UTIL_SIGNALS_H_
