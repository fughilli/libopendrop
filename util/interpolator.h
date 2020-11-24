#ifndef UTIL_INTERPOLATOR_H_
#define UTIL_INTERPOLATOR_H_

#include "libopendrop/util/logging.h"

namespace opendrop {

template <typename T>
class Interpolator;

template <typename T>
class InterpolatorIterator {
 public:
  enum State {
    kValid = 0,
    kEnd,
  };

  InterpolatorIterator(const Interpolator<T>* interpolator, T value,
                       State state)
      : interpolator_(interpolator), value_(value), state_(state) {}

  InterpolatorIterator(const Interpolator<T>* interpolator, T value)
      : InterpolatorIterator(interpolator, value, State::kValid) {}

  InterpolatorIterator() : InterpolatorIterator(nullptr, 0, State::kEnd) {}

  T operator*() const { return value_; }

  InterpolatorIterator<T> operator++(int) {
    if (state_ == kEnd || value_ == interpolator_->end_value()) {
      state_ = kEnd;
      return *this;
    }

    InterpolatorIterator<T> return_value = *this;
    value_ += interpolator_->step();

    if (interpolator_->step() > 0) {
      if (value_ > interpolator_->end_value()) {
        value_ = interpolator_->end_value();
      }
    } else {
      if (value_ < interpolator_->end_value()) {
        value_ = interpolator_->end_value();
      }
    }

    return return_value;
  }

  InterpolatorIterator<T>& operator++() {
    if (state_ == kEnd || value_ == interpolator_->end_value()) {
      state_ = kEnd;
      return *this;
    }

    value_ += interpolator_->step();

    if (interpolator_->step() > 0) {
      if (value_ > interpolator_->end_value()) {
        value_ = interpolator_->end_value();
      }
    } else {
      if (value_ < interpolator_->end_value()) {
        value_ = interpolator_->end_value();
      }
    }

    return *this;
  }

  bool operator==(const InterpolatorIterator<T>& other) const {
    return !operator!=(other);
  }

  bool operator!=(const InterpolatorIterator<T>& other) const {
    // If the iterators are in different states, then they are not equal.
    if (state_ != other.state_) {
      return true;
    }

    // If the iterators are in the same state, and it's the end state, then they
    // are equal.
    if (state_ == kEnd) {
      return false;
    }

    // Otherwise, compare the value.
    return value_ != other.value_;
  }

 private:
  const Interpolator<T>* interpolator_;
  T value_;
  State state_;
};

template <typename T>
class Interpolator {
 public:
  Interpolator(T begin_value, T end_value, T step)
      : begin_value_(begin_value), end_value_(end_value), step_(step) {
    if (std::abs(step_) < kEpsilon) {
      if (end_value >= begin_value) {
        step_ = kEpsilon;
      } else {
        step = -kEpsilon;
      }
    }
    CHECK((step > 0) ? (end_value >= begin_value) : (end_value <= begin_value))
        << "The sign of the step must be the same as the sign of the "
           "expression `(end_value - begin_value)`. [end_value: "
        << end_value << ", begin_value: " << begin_value << ", step: " << step;
  }

  Interpolator() : Interpolator(0, 0, 1) {}

  static Interpolator<T> WithStepCount(T begin_value, T end_value,
                                       int step_count) {
    T total_distance = (end_value - begin_value);
    T step = 0;
    if (begin_value == end_value) {
      step = 0;
    } else {
      CHECK(step_count > 0) << "Cannot initialize an Interpolator with 0 steps "
                               "if begin_value != end_value";
      step = total_distance / step_count;
    }

    return Interpolator(begin_value, end_value, step);
  }

  T begin_value() const { return begin_value_; }
  T end_value() const { return end_value_; }
  T step() const { return step_; }

  InterpolatorIterator<T> begin() const {
    return InterpolatorIterator<T>(this, begin_value_);
  }

  InterpolatorIterator<T> end() const {
    return InterpolatorIterator<T>(this, end_value_,
                                   InterpolatorIterator<T>::State::kEnd);
  }

 private:
  static constexpr float kEpsilon = 1e-6f;
  T begin_value_;
  T end_value_;
  T step_;
};

}  // namespace opendrop

#endif  // UTIL_INTERPOLATOR_H_
