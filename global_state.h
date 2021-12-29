#ifndef GLOBAL_STATE_H_
#define GLOBAL_STATE_H_

#include "absl/types/span.h"
#include "util/accumulator.h"
#include "util/filter.h"

namespace opendrop {

class GlobalState {
 public:
  struct Options {
    int sampling_rate;
  };

  GlobalState(Options options)
      : options_(std::move(options)), initialized_(false) {}

  // Updates the global state. `samples` is a const view of the current buffer
  // of audio samples, and `dt` is the elapsed time, in seconds, since the last
  // time `Update` was invoked.
  void Update(absl::Span<const float> samples, float dt);

  // Accessors for global state properties.
  float t() { return properties_.time; }
  float dt() { return properties_.dt; }
  float power() { return properties_.power; }
  float average_power() { return properties_.average_power; }
  Accumulator<float>& energy() { return properties_.energy; }
  Accumulator<float>& normalized_energy() {
    return properties_.normalized_energy;
  }

  int sampling_rate() const { return options_.sampling_rate; }

  absl::Span<const float> left_channel() { return left_channel_; }
  absl::Span<const float> right_channel() { return right_channel_; }

 private:
  // Decay factor for updating the average power. Average power is computed by a
  // first-order low-pass filter of the current signal power.
  float kPowerUpdateAlpha = 0.99f;

  // Decay factor for initializing the average power. This should be
  // significantly less than 1, such that the average power quickly converges to
  // the order of magnitude of the power at initialization.
  float kPowerInitializationUpdateAlpha = 0.8f;

  struct Properties {
    Properties() : dt(0), time(0), power(0), average_power(0) {
      energy.SetValue(0);
      normalized_energy.SetValue(0);
    }

    // Elapsed time, in seconds, since last frame.
    float dt;

    // Time since program start, in seconds.
    float time;

    // Power of last buffer of audio samples.
    float power;

    // Integral of audio signal power over time.
    Accumulator<float> energy;

    // Average power over time.
    float average_power;

    // Normalized energy (energy accumulated as \int{power / average_power}).
    Accumulator<float> normalized_energy;
  };

  Options options_;

  bool initialized_;
  IirFilter average_power_initialization_filter_{
      {1 - kPowerInitializationUpdateAlpha}, {kPowerInitializationUpdateAlpha}};
  IirFilter average_power_filter_{{1 - kPowerUpdateAlpha}, {kPowerUpdateAlpha}};
  int average_power_initialization_filter_samples_ = 0;

  // Storage for global properties.
  Properties properties_;

  std::vector<float> left_channel_;
  std::vector<float> right_channel_;
};

}  // namespace opendrop

#endif  // GLOBAL_STATE_H_
