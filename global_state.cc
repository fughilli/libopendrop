#include "libopendrop/global_state.h"

#include <cmath>

namespace opendrop {

namespace {

// Very small floating point value.
float kEpsilon = 1e-12f;

}  // namespace

void GlobalState::Update(absl::Span<const float> samples, float dt) {
  if ((samples.size() / 2) != left_channel_.size()) {
    left_channel_.resize(samples.size() / 2, 0);
    right_channel_.resize(samples.size() / 2, 0);
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
      left_channel_[i / 2] = samples[i];
    } else {
      right_channel_[i / 2] = samples[i];
    }
    properties_.power += std::pow(samples[i], 2);
  }
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
