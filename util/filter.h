#ifndef UTIL_FIR_FILTER_H_
#define UTIL_FIR_FILTER_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "absl/types/span.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

class Filter {
 public:
  virtual float ProcessSample(float sample) = 0;

  float ComputePower(absl::Span<const float> samples);
};

// Finite impulse response time-domain convolutional filter.
class FirFilter : public Filter {
 public:
  // Constructs a finite impulse response filter with the provided taps.
  FirFilter(std::initializer_list<float> taps);

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
            std::initializer_list<float> y_taps);

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

enum IirBandFilterType {
  // Bandpass filter.
  kBandpass = 0,
  // Bandstop filter.
  kBandstop
};
// Initializes and returns an infinite impulse response filter implementing a
// 2-pole bandpass or bandstop filter
std::shared_ptr<IirFilter> IirBandFilter(float center_frequency,
                                         float bandwidth,
                                         IirBandFilterType type);

// Implements a hysteretic "map" filter. This filter takes a time-varying
// signal, computes its maxima and minima with a decay towards its low-passed
// value (average), and outputs a value in the range [0.0, 1.0].
class HystereticMapFilter : public Filter {
 public:
  // Constructs a HystereticMapFilter using `averaging_filter` to compute the
  // signal average, towards which the computed signal minima and maxima will
  // exponentially decay towards by the coefficient `alpha`.
  HystereticMapFilter(FirFilter averaging_filter, float alpha)
      : averaging_filter_(std::move(averaging_filter)),
        alpha_(std::clamp<float>(alpha, 0.0f, 1.0f)) {}

  virtual float ProcessSample(float sample) override;

 private:
  FirFilter averaging_filter_;
  const float alpha_;

  float maximum_ = std::numeric_limits<float>::lowest();
  float minimum_ = std::numeric_limits<float>::max();
};

}  // namespace opendrop

#endif  // UTIL_FIR_FILTER_H_
