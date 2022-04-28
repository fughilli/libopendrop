#ifndef UTIL_SIGNAL_DECAY_TOWARDS_H_
#define UTIL_SIGNAL_DECAY_TOWARDS_H_

#include <algorithm>

namespace opendrop {

class DecayTowards {
 public:
  DecayTowards(float alpha) : alpha_(alpha) {}

  float& value() { return value_; }

  // Exponentially decays the value stored towards `towards` by the configured
  // alpha.
  //
  // TODO: compensate by dt
  void Decay(float towards, float strength = 1.0f) {
    float direction = towards - value_;
    value_ += direction * alpha_ * std::clamp(strength, 0.0f, 1.0f);
  }

 private:
  float value_ = 0.0f;
  float alpha_;
};

}  // namespace opendrop

#endif  // UTIL_SIGNAL_DECAY_TOWARDS_H_
