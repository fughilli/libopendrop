#ifndef UTIL_AUDIO_BEAT_ESTIMATOR_H_
#define UTIL_AUDIO_BEAT_ESTIMATOR_H_

#include <algorithm>
#include <cmath>
#include <numeric>
#include <ostream>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "util/logging/logging.h"
#include "util/math/math.h"

namespace opendrop {

class BeatEstimator {
 public:
  BeatEstimator(float alpha) : alpha_(alpha) {}

  const BeatEstimator& Estimate(float signal, float dt) {
    UpdateCounts();

    signal = std::abs(signal);

    bool is_beat_current = (signal > threshold_ * kThresholdFraction);

    if (signal > threshold_) {
      threshold_ = signal;
    } else {
      threshold_ = threshold_ * alpha_ + signal * (1.0f - alpha_);
    }

    if (!is_beat_ && is_beat_current && count_ > kCooldownTime) {
      triangle_phase_last_beat_value_ = triangle_phase();
      triangle_phase_target_value_ = 1 - triangle_phase_target_value_;

      // Rising edge; push non-beat onset sample count.
      PushCount();
      count_ = 0;

      duration_ = AverageMaxCount();
    } else {
      count_ += dt;
    }

    is_beat_ = is_beat_current;

    is_binned_beat_ = count_ == duration_;

    return *this;
  }

  bool beat() const { return is_beat_; }

  float phase() const {
    if (duration_ == 0) {
      return 0;
    }
    return std::clamp(static_cast<float>(count_) / duration_, 0.0f, 1.0f);
  }

  float triangle_phase() const {
    return Lerp(triangle_phase_last_beat_value_, triangle_phase_target_value_,
                phase());
  }

  bool beat_binned() const { return is_binned_beat_; }

  void Print() const {
    for (auto& [k, v] : bin_counts_) {
      LOG(INFO) << absl::StrFormat("BIN[%f] = {%s}", k, v.Stringify());
    }
  }

  float threshold() const { return threshold_; }

 private:
  static constexpr float kThresholdFraction = 0.8;
  static constexpr int kNumEntries = 10;
  static constexpr int kCooldownTime = 1 / 200.0;

  float AverageMaxCount() {
    auto& [max_key, max_value] =
        *std::max_element(bin_counts_.begin(), bin_counts_.end(),
                          [](const auto& a_kv, const auto& b_kv) -> bool {
                            auto& [a_key, a_value] = a_kv;
                            auto& [b_key, b_value] = b_kv;
                            return a_value.score < b_value.score;
                          });
    return std::accumulate(max_value.counts.begin(), max_value.counts.end(),
                           0.0f) /
           max_value.counts.size();
  }

  void PushCount() {
    auto& entry = bin_counts_[GetBin(count_)];
    entry.score += 1.0f;
    entry.counts[entry.head] = count_;
    entry.head = (entry.head + 1) % kNumEntries;
    count_ = 0;
  }

  void UpdateCounts() {
    for (auto& [key, value] : bin_counts_) {
      value.score *= alpha_;
    }
  }

  int GetBin(float count) { return Quantize(count, 0.001f); }

  struct Entry {
    float score;
    std::vector<float> counts;
    int head;

    Entry() : score(0), counts(kNumEntries, 0), head(0) {}

    std::string Stringify() const {
      return absl::StrFormat(
          "{.score = %f, .counts = %s}", score,
          ToString(absl::Span<const float>(counts.data(), counts.size())));
    }
  };

  absl::flat_hash_map<float, Entry> bin_counts_{};

  float count_ = 0;
  float duration_ = 0;
  float alpha_ = 0.0f;
  float threshold_ = 0.0f;
  bool is_beat_ = false;
  bool is_binned_beat_ = false;

  float triangle_phase_value_ = 0;
  float triangle_phase_target_value_ = 0;
  float triangle_phase_last_beat_value_ = 0;
};

std::ostream& operator<<(std::ostream& os, const std::vector<int>& ints);

}  // namespace opendrop

#endif  // UTIL_AUDIO_BEAT_ESTIMATOR_H_
