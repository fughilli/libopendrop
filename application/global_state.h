#ifndef APPLICATION_GLOBAL_STATE_H_
#define APPLICATION_GLOBAL_STATE_H_

#include <array>
#include <vector>

#include "absl/types/span.h"
#include "util/signal/accumulator.h"
#include "util/signal/filter.h"
#include "util/signal/unitizer.h"

namespace opendrop {

class GlobalState {
 public:
  static constexpr int kNumFilterBands = 3;

  struct Options {
    int sampling_rate;
  };

  GlobalState(Options options);

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

  absl::Span<const float> left_channel() { return channels_[0]; }
  absl::Span<const float> right_channel() { return channels_[1]; }

  float bass_left() const { return channel_bands_[0][0]; }
  float bass_right() const { return channel_bands_[1][0]; }
  float mid_left() const { return channel_bands_[0][1]; }
  float mid_right() const { return channel_bands_[1][1]; }
  float treble_left() const { return channel_bands_[0][2]; }
  float treble_right() const { return channel_bands_[1][2]; }

  float bass() const { return bass_left() + bass_right(); }
  float mid() const { return mid_left() + mid_right(); }
  float treble() const { return treble_left() + treble_right(); }

  float bass_u() const { return bass_u_; }
  float mid_u() const { return mid_u_; }
  float treble_u() const { return treble_u_; }

  float bass_left_energy() const { return channel_bands_energy_[0][0]; }
  float bass_right_energy() const { return channel_bands_energy_[1][0]; }
  float mid_left_energy() const { return channel_bands_energy_[0][1]; }
  float mid_right_energy() const { return channel_bands_energy_[1][1]; }
  float treble_left_energy() const { return channel_bands_energy_[0][2]; }
  float treble_right_energy() const { return channel_bands_energy_[1][2]; }

  float bass_energy() const { return bass_left_energy() + bass_right_energy(); }
  float mid_energy() const { return mid_left_energy() + mid_right_energy(); }
  float treble_energy() const {
    return treble_left_energy() + treble_right_energy();
  }

  float channel_band_left(int band) const {
    return channel_bands_[0][std::clamp(band, 0, kNumFilterBands)];
  }
  float channel_band_right(int band) const {
    return channel_bands_[1][std::clamp(band, 0, kNumFilterBands)];
  }

  float channel_band(int band) const {
    return channel_band_left(band) + channel_band_right(band);
  }

 private:
  static constexpr int kNumChannels = 2;
  using FilterCoeffs = std::tuple<float, float, IirBandFilterType>;
  static constexpr std::array<FilterCoeffs, kNumFilterBands> kFilterBandCoeffs =
      {
          FilterCoeffs{20, 300, IirBandFilterType::kBandpass},
          FilterCoeffs{300, 4000, IirBandFilterType::kBandpass},
          FilterCoeffs{4000, 15000, IirBandFilterType::kBandpass},
  };

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

  std::array<std::vector<float>, kNumChannels> channels_ = {};
  std::array<std::array<std::shared_ptr<IirFilter>, kNumFilterBands>,
             kNumChannels>
      channel_band_filters_ = {};
  std::array<std::array<float, kNumFilterBands>, kNumChannels> channel_bands_ =
      {};
  std::array<std::array<float, kNumFilterBands>, kNumChannels>
      channel_bands_energy_ = {};

  Unitizer bass_unitizer_{Unitizer::Options{}};
  Unitizer mid_unitizer_{Unitizer::Options{}};
  Unitizer treble_unitizer_{Unitizer::Options{}};
  float bass_u_ = 0;
  float mid_u_ = 0;
  float treble_u_ = 0;
};

}  // namespace opendrop

#endif  // APPLICATION_GLOBAL_STATE_H_
