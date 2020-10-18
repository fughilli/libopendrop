#include "libopendrop/global_state.h"

#include <cmath>

namespace opendrop {

namespace {
// Decay factor for updating the average power. Average power is computed by a
// first-order low-pass filter of the current signal power.
float kPowerUpdateAlpha = 0.99f;

// Very small floating point value.
float kEpsilon = 1e-12f;

}  // namespace

void GlobalState::Update(absl::Span<const float> samples, float dt) {
  // TODO: Test that these values trend the same way across framerates.
  // TODO: Implement using Eigen.
  properties_.dt = dt;
  properties_.time += dt;

  properties_.power = 0;
  // Note that this buffer is interleaved samples. Computing the power assuming
  // this is a mono buffer has the same outcome as averaging the power of the
  // left and right channels independently.
  for (auto sample : samples) {
    properties_.power += std::pow(sample, 2);
  }
  if (samples.size() != 0) {
    properties_.power /= samples.size();
  }

  properties_.energy += properties_.power * dt;

  properties_.average_power = properties_.average_power * kPowerUpdateAlpha +
                              properties_.power * (1.0f - kPowerUpdateAlpha);

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
