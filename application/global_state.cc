#include "application/global_state.h"

#include <array>
#include <cmath>
#include <utility>
#include <vector>

#include "absl/types/span.h"

namespace opendrop {

namespace {

// Very small floating point value.
float kEpsilon = 1e-12f;

}  // namespace

GlobalState::GlobalState(GlobalState::Options options)
    : options_(std::move(options)), initialized_(false) {
  for (int channel = 0; channel < kNumChannels; ++channel) {
    for (int band = 0; band < kNumFilterBands; ++band) {
      const auto [low_freq, high_freq, filter_type] = kFilterBandCoeffs[band];
      const float center_freq = (low_freq + high_freq) / 2;
      const float bandwidth = std::abs(high_freq - low_freq);
      channel_band_filters_[channel][band] = IirBandFilter(
          center_freq / 44100.0f, bandwidth / 44100.0f, filter_type);
    }
  }
}

void GlobalState::Update(absl::Span<const float> samples, float dt) {
  if ((samples.size() / 2) != channels_[0].size()) {
    channels_[0].resize(samples.size() / 2, 0);
    channels_[1].resize(samples.size() / 2, 0);
  }
  // TODO: Test that these values trend the same way across framerates.
  // TODO: Implement using Eigen.
  properties_.dt = dt;
  properties_.time += dt;

  properties_.power = 0;
  // Note that this buffer is interleaved samples. Computing the power assuming
  // this is a mono buffer has the same outcome as averaging the power of the
  // left and right channels independently.
  for (int i = 0; i < samples.size(); ++i) {
    if (i % 2 == 0) {
      channels_[0][i / 2] = samples[i];
    } else {
      channels_[1][i / 2] = samples[i];
    }
    properties_.power += std::pow(samples[i], 2);
  }

  for (int channel = 0; channel < kNumChannels; ++channel) {
    for (int band = 0; band < kNumFilterBands; ++band) {
      float value = channel_band_filters_[channel][band]->ComputePower(
          channels_[channel]);
      if (std::isnan(value)) continue;
      channel_bands_[channel][band] = value;
      channel_bands_energy_[channel][band] +=
          channel_bands_[channel][band] * dt;
    }
  }

  bass_u_ = bass_unitizer_.Update(bass());
  mid_u_ = bass_unitizer_.Update(mid());
  treble_u_ = bass_unitizer_.Update(treble());

  if (samples.size() != 0) {
    properties_.power = properties_.power / samples.size();
  }

  properties_.energy += properties_.power * dt;

  if (average_power_initialization_filter_samples_ < 100) {
    ++average_power_initialization_filter_samples_;
    float out_sample =
        average_power_initialization_filter_.ProcessSample(properties_.power);
    if (average_power_initialization_filter_samples_ == 100) {
      properties_.average_power = out_sample;
    }
  } else {
    properties_.average_power =
        average_power_filter_.ProcessSample(properties_.power);
  }

  // Wait for average power to increase beyond epsilon before starting to
  // compute the normalized energy.
  if (initialized_ || abs(properties_.average_power) >= kEpsilon) {
    initialized_ = true;
    // Disallow division by zero.
    if (properties_.average_power != 0.0f) {
      properties_.normalized_energy +=
          properties_.power / properties_.average_power;
    }
  }
}

}  // namespace opendrop
