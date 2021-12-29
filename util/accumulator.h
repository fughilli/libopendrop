#ifndef UTIL_ACCUMULATOR_H_
#define UTIL_ACCUMULATOR_H_

#include <cmath>

#include "util/interpolator.h"

namespace opendrop {

template <typename T>
class Accumulator {
 public:
  Accumulator() : value_(0), is_periodic_(false), period_(0), last_step_(0) {}

  Accumulator& SetValue(T value) {
    value_ = value;
    return *this;
  }

  Accumulator& SetPeriod(T period) {
    is_periodic_ = true;
    period_ = period;
    return *this;
  }

  // Update this accumulator with the given value.
  T Update(T step) {
    last_step_ = step;
    value_ += step;

    if (is_periodic_ && value_ > period_) {
      value_ -= std::floor(value_ / period_) * period_;
    }

    return value_;
  }

  Interpolator<T> InterpolateLastStep(T step_size) const {
    T last_value = value_ - last_step_;

    return Interpolator<T>(last_value, value_, step_size);
  }

  Interpolator<T> InterpolateLastStepWithStepCount(int step_count) const {
    T last_value = value_ - last_step_;
    return Interpolator<T>::WithStepCount(last_value, value_, step_count);
  }

  T value() const { return value_; }
  T last_step() const { return last_step_; }

  // Syntactic sugar.
  T operator+=(T step) { return Update(step); }
  operator T() const { return value_; }

 private:
  T value_;
  bool is_periodic_;
  T period_;

  T last_step_;
};

}  // namespace opendrop

#endif  // UTIL_ACCUMULATOR_H_
