#ifndef DEBUG_SIGNAL_SCOPE_H_
#define DEBUG_SIGNAL_SCOPE_H_

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "implot.h"
#include "util/logging/logging.h"
#include "util/math/math.h"

namespace opendrop {

class SignalScope {
 public:
  static void Plot() {
    auto& ss = instance();
    ImPlot::SetNextAxisToFit(ImAxis_X1);
    ImPlot::SetNextAxisLimits(ImAxis_Y1, -1.0f, 1.0f);
    if (ImPlot::BeginPlot("signals", ImVec2(-1, -1))) {
      ImPlot::SetupLegend(ImPlotLocation_NorthWest, ImPlotLegendFlags_Outside);
      for (auto& [name, signal] : ss.signals_by_name_) {
        ImPlot::HideNextItem(!signal.draw_by_default);
        ImPlot::PlotLine(name.c_str(), signal.signal.data(),
                         signal.signal.size());
        std::copy(signal.signal.begin() + 1, signal.signal.end(),
                  signal.signal.begin());
      }
      ImPlot::EndPlot();
    }
  }

  template <typename T>
  static T PlotSignal(absl::string_view name, T value) {
    instance().PlotSignalInternal(name, value);
    return value;
  }

  template <typename T>
  static T PlotSignalOn(absl::string_view name, T value) {
    instance().PlotSignalInternal(name, value, true);
    return value;
  }

  template <typename T>
  static T PlotSignalClamp(absl::string_view name, T value, T low, T high) {
    instance().PlotSignalInternal(name, std::clamp(value, low, high));
    return value;
  }

  template <typename T>
  static T PlotSignalWrap(absl::string_view name, T value, T low, T high) {
    instance().PlotSignalInternal(name, WrapToRange(value, low, high));
    return value;
  }

 private:
  static constexpr size_t kDefaultHistorySize = 1024;

  struct Signal {
    std::vector<float> signal;
    bool draw_by_default = false;
  };

  static SignalScope& instance() {
    static SignalScope* instance = nullptr;

    if (instance != nullptr) return *instance;

    instance = new SignalScope();
    return *instance;
  }

  void PlotSignalInternal(absl::string_view name, float value,
                          bool draw_by_default = false) {
    auto iter = signals_by_name_.find(name);
    if (iter == signals_by_name_.end()) {
      auto iter_and_success = signals_by_name_.try_emplace(
          name, Signal{.signal = std::vector<float>(kDefaultHistorySize),
                       .draw_by_default = draw_by_default});
      CHECK(iter_and_success.second);
      auto& signal = iter_and_success.first->second;
      signal.signal[signal.signal.size() - 1] = value;
      return;
    }

    auto& signal = iter->second;
    signal.signal[signal.signal.size() - 1] = value;
  }

  absl::flat_hash_map<std::string, Signal> signals_by_name_;
};

// Plots a signal named `name` of the time history of `value`.
#define SIGPLOT(name, value) SignalScope::PlotSignal((name), (value))
#define SIGPLOT_ON(name, value) SignalScope::PlotSignalOn((name), (value))

#define XSTR(X) #X
#define STR(X) XSTR(X)
// Plots the time history of `value`, automatically assigning a name.
#define SIGPLOT_AUTO(value)                                                 \
  SignalScope::PlotSignal(("" __FILE__ ":" STR(__LINE__) "| " #value " |"), \
                          (value))

// Same as above, but wraps `value` to the range given by [`low`, `high`).
#define SIGPLOT_WRAP(name, value, low, high) \
  SignalScope::PlotSignalWrap((name), (value), (low), (high))
#define SIGPLOT_AUTO_WRAP(value, low, high)                                 \
  SignalScope::PlotSignal(("" __FILE__ ":" STR(__LINE__) "| " #value " |"), \
                          (value), (low), (high))

// Declares a local variable and plots its time history.
//
// Declares a local variable named `name` and assigns `value` to it. Plots the
// time history of `value`, assigning `name` to it.
#define SIGPLOT_ASSIGN(name, value) \
  name = SignalScope::PlotSignal(("" #name), (value))
#define SIGPLOT_ASSIGN_WRAP(name, value, low, high) \
  name = SignalScope::PlotSignalWrap(("" #name), (value), (low), (high))

}  // namespace opendrop

#endif  // DEBUG_SIGNAL_SCOPE_H_
