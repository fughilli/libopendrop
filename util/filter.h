#ifndef UTIL_FIR_FILTER_H_
#define UTIL_FIR_FILTER_H_

#include <algorithm>
#include <vector>

#include "absl/types/span.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

class Filter {
 public:
  virtual float ProcessSample(float sample) = 0;

  float ComputePower(absl::Span<const float> samples) {
    float power = 0.0f;
    for (auto sample : samples) {
      float output_sample = ProcessSample(sample);
      power += output_sample * output_sample;
    }
    return power / samples.size();
  }
};

// Finite impulse response time-domain convolutional filter.
class FirFilter : public Filter {
 public:
  // Constructs a finite impulse response filter with the provided taps.
  FirFilter(std::initializer_list<float> taps) : taps_(taps) {
    input_history_.resize(taps.size(), 0);
  }

  virtual float ProcessSample(float sample) override {
    std::copy_backward(input_history_.begin(), std::prev(input_history_.end()),
                       input_history_.end());
    input_history_[0] = sample;

    float output = 0.0f;
    for (int i = 0; i < input_history_.size(); ++i) {
      output += input_history_[i] * taps_[i];
    }

    return output;
  }

  // Processes a single sample by this filter, returning the corresponding
  // output sample.
  virtual float ProcessSample(float sample) override;

 private:
  // The time-domain filter taps for this filter.
  std::vector<float> taps_;
  // The time history of the input signal. New elements are shifted onto the
  // front of the buffer; older elements are at the end.
  std::vector<float> input_history_;
};

// Infinite impulse response time-domain convolutional filter.
class IirFilter : public Filter {
 public:
  // Constructs an infinite impulse response filter with the provided taps.
  // `x_taps` are the coefficients for the input signal; `y_taps` are the
  // feedback coefficients for the output signal.
  IirFilter(std::initializer_list<float> x_taps,
            std::initializer_list<float> y_taps)
      : x_taps_(x_taps), y_taps_(y_taps) {
    input_history_.resize(x_taps.size(), 0);
    output_history_.resize(y_taps.size(), 0);
  }

  virtual float ProcessSample(float sample) override {
    std::copy_backward(input_history_.begin(), std::prev(input_history_.end()),
                       input_history_.end());
    input_history_[0] = sample;

    float output = 0.0f;
    for (int i = 0; i < input_history_.size(); ++i) {
      output += input_history_[i] * x_taps_[i];
    }

    for (int i = 0; i < output_history_.size(); ++i) {
      output += output_history_[i] * y_taps_[i];
    }

    std::copy_backward(output_history_.begin(),
                       std::prev(output_history_.end()), output_history_.end());
    output_history_[0] = output;

    return output;
  }

  // Processes a single sample by this filter, returning the corresponding
  // output sample.
  virtual float ProcessSample(float sample) override;

 private:
  // Time-domain taps and history buffer for the input signal. Ordering is the
  // same as in `FirFilter`.
  std::vector<float> x_taps_;
  std::vector<float> input_history_;

  // Time-domain taps and history buffer for the output signal. Note that the
  // 0th index tap in `y_taps_` is multiplied with the immediately *previous*
  // output of the filter.
  std::vector<float> y_taps_;
  std::vector<float> output_history_;
};

}  // namespace opendrop

#endif  // UTIL_FIR_FILTER_H_
