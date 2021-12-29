#ifndef LIBOPENDROP_NORMALIZER_H_
#define LIBOPENDROP_NORMALIZER_H_

#include "absl/types/span.h"
#include "util/logging.h"

namespace opendrop {

// This class implements an audio sample normalizer. The normalizer maintains a
// normalization coefficient which is determined by selecting the maximum sample
// value of each incoming buffer of audio samples, and applying a simple
// low-pass filter to it.
class Normalizer {
 public:
  // Constructs a Normalizer with the given low-pass-filter coefficient and
  // instant upscaling configuration value.
  Normalizer(float alpha, bool instant_upscale)
      : alpha_(alpha),
        instant_upscale_(instant_upscale),
        normalization_divisor_(1.0f) {
    CHECK(alpha >= 0.0f && alpha <= 1.0f)
        << "alpha must be between 0.0 and 1.0";
  }

  // Normalizes `samples`, writing the resulting normalized values into
  // `out_samples`.
  void Normalize(absl::Span<const float> samples, float dt,
                 absl::Span<float> out_samples) {
    CHECK(samples.size() == out_samples.size())
        << "Input and output buffers must be the same size";
    float max_value = 0.0f;
    for (auto sample : samples) {
      max_value = std::max(max_value, std::abs(sample));
    }

    if (instant_upscale_ && max_value > normalization_divisor_) {
      normalization_divisor_ = max_value;
    } else {
      float alpha_compensated = alpha_;
      normalization_divisor_ = normalization_divisor_ * alpha_compensated +
                               max_value * (1.0f - alpha_compensated);
    }

    float normalization_factor = 1.0f / std::max(normalization_divisor_, 0.01f);
    for (int i = 0; i < samples.size(); ++i) {
      out_samples[i] = samples[i] * normalization_factor;
    }
  }

 private:
  // Normalization divisor low-pass-filter coefficient.
  float alpha_;

  // Whether or not to instantly increase the normalization divisor when the
  // current frame max value is greater than the current normalization divisor
  // value.
  bool instant_upscale_;

  // Current normalization divisor value.
  float normalization_divisor_;
};
}  // namespace opendrop

#endif  // LIBOPENDROP_NORMALIZER_H_
