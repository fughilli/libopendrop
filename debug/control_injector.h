#ifndef DEBUG_CONTROL_INJECTOR_H_
#define DEBUG_CONTROL_INJECTOR_H_

#include <limits>
#include <set>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "debug/control.pb.h"
#include "debug/proto_port.h"
#include "implot.h"
#include "util/logging.h"
#include "util/math.h"

namespace opendrop {

class ControlInjector {
 public:
  static void Inject() { instance().InjectHelper(); }

  static void UpdateControl(absl::string_view name, float value) {
    auto& ss = instance();
    auto iter = ss.controls_by_name_.find(name);
    if (iter == ss.controls_by_name_.end()) {
      auto iter_and_success = ss.controls_by_name_.try_emplace(name, value);
      CHECK(iter_and_success.second);
      return;
    }

    iter->second = value;
  }

  template <typename T>
  static T InjectEnum(absl::string_view name, T value) {
    return static_cast<T>(InjectCounter(name, static_cast<int>(value), 0,
                                        static_cast<int>(T::kDenseLastValue)));
  }

  static bool InjectTrigger(absl::string_view name) {
    return instance().InjectTriggerInternal(name);
  }

  template <typename T>
  static T InjectCounter(absl::string_view name, T value, T low, T high) {
    return instance().InjectCounterInternal(name, value, low, high);
  }

  template <typename T>
  static T InjectSignal(absl::string_view name, T value) {
    return instance().InjectSignalInternal(name, value);
  }

  template <typename T>
  static T InjectSignalClamp(absl::string_view name, T value, T low, T high) {
    const float interpolator_clamped =
        MapValue<float, /*clamp=*/true>(value, low, high, 0.0f, 1.0f);
    return MapValue<float>(
        instance().InjectSignalInternal(name, interpolator_clamped), 0.0f, 1.0f,
        low, high);
  }

  template <typename T>
  static T InjectSignalOverride(absl::string_view name, T value, T low,
                                T high) {
    return instance().InjectSignalInternal(name, value, low, high);
  }

 private:
  static constexpr size_t kDefaultHistorySize = 1024;

  struct Signal {
    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::lowest();

    void UpdateLimits(float value) {
      if (value < min) min = value;
      if (value > max) max = value;
    }

    void UpdateLimits(float low, float high) {
      min = low;
      max = high;
    }

    float Inject(float control) { return Lerp(min, max, control); }
  };

  struct Counter {
    int min;
    int max;

    int value;
    bool triggered;

    void Increment() {
      triggered = true;
      ++value;

      if (value > max) {
        value = min;
      }
    }

    bool WasTriggered() {
      if (!triggered) return false;
      triggered = false;
      return true;
    }
  };

  ControlInjector() : control_port_(9944) {}

  static ControlInjector& instance() {
    static ControlInjector* instance = nullptr;

    if (instance != nullptr) return *instance;

    instance = new ControlInjector();
    return *instance;
  }

  void MaybeUpdateButtons(int button) {
    if (buttons_.count(button)) return;

    buttons_.insert(button);

    buttons_as_strings_.clear();
    buttons_as_ints_.clear();
    for (int button : buttons_) {
      buttons_as_ints_.push_back(button);
      buttons_as_strings_.push_back(absl::StrFormat("%d", button));
    }
  }

  void InjectHelper() {
    Control control{};
    while (control_port_.Read(&control)) {
      for (auto& [control_name, control_value] : control.control()) {
        controls_by_name_[control_name] = control_value;
      }

      for (auto& button : control.buttons()) {
        if (button.state() != Button::RELEASED) continue;

        MaybeUpdateButtons(button.channel());

        for (auto& [counter_name, counter] : counters_by_name_) {
          const auto& button_mapping = button_mappings_.find(counter_name);
          if (button_mapping == button_mappings_.end()) continue;

          if (button_mapping->second == button.channel()) counter.Increment();
        }
      }
    }

    std::vector<const char*> controls = {};
    for (auto& [control_name, control_value] : controls_by_name_)
      controls.push_back(control_name.c_str());

    ImGui::Checkbox("Signal Inject Enable?", &enable_injection_);

    for (auto& [signal_name, signal_value] : signals_by_name_) {
      int selection = -1;
      ImGui::Combo(signal_name.c_str(), &selection, controls.data(),
                   controls.size());
      if (selection != -1) {
        control_mappings_[signal_name] = controls[selection];
      }
    }

    std::vector<const char*> buttons = {};
    for (auto& str_button : buttons_as_strings_)
      buttons.push_back(str_button.c_str());

    for (auto& [counter_name, counter] : counters_by_name_) {
      int selection = -1;
      ImGui::Combo(counter_name.c_str(), &selection, buttons.data(),
                   buttons.size());
      if (selection != -1) {
        button_mappings_[counter_name] = buttons_as_ints_[selection];
      }
    }

    std::vector<std::string> to_delete = {};
    for (auto& [signal_name, control_name] : control_mappings_) {
      std::string mapping_label =
          absl::StrFormat("%s -> %s", signal_name, control_name);
      if (ImGui::SmallButton(mapping_label.c_str())) {
        to_delete.push_back(signal_name);
      }
    }

    for (auto& name : to_delete) control_mappings_.erase(name);

    to_delete = {};
    for (auto& [counter_name, button] : button_mappings_) {
      std::string mapping_label =
          absl::StrFormat("%s -> %d", counter_name, button);
      if (ImGui::SmallButton(mapping_label.c_str())) {
        to_delete.push_back(counter_name);
      }
    }

    for (auto& name : to_delete) button_mappings_.erase(name);
  }

  int InjectCounterInternal(absl::string_view name, int value, int low,
                            int high) {
    auto iter = counters_by_name_.find(name);
    if (iter == counters_by_name_.end()) {
      auto iter_and_success = counters_by_name_.try_emplace(
          name, Counter{.min = low, .max = high, .value = value});
      CHECK(iter_and_success.second);
      return value;
    }

    auto& counter = iter->second;

    if (!enable_injection_) {
      counter.value = value;
      return value;
    }

    return counter.value;
  }

  bool InjectTriggerInternal(absl::string_view name) {
    auto iter = counters_by_name_.find(name);
    if (iter == counters_by_name_.end()) {
      auto iter_and_success = counters_by_name_.try_emplace(name, Counter{});
      CHECK(iter_and_success.second);
      return false;
    }

    auto& counter = iter->second;

    if (!enable_injection_) {
      return false;
    }

    return counter.WasTriggered();
  }

  float InjectSignalInternal(
      absl::string_view name, float value,
      float low = std::numeric_limits<float>::quiet_NaN(),
      float high = std::numeric_limits<float>::quiet_NaN()) {
    auto iter = signals_by_name_.find(name);
    if (iter == signals_by_name_.end()) {
      auto iter_and_success = signals_by_name_.try_emplace(name, Signal{});
      CHECK(iter_and_success.second);
      auto& signal = iter_and_success.first->second;
      if (std::isnan(low) || std::isnan(high))
        signal.UpdateLimits(value);
      else
        signal.UpdateLimits(low, high);
      return value;
    }

    auto& signal = iter->second;
    if (std::isnan(low) || std::isnan(high))
      signal.UpdateLimits(value);
    else
      signal.UpdateLimits(low, high);

    if (!enable_injection_) return value;

    auto mapping_iter = control_mappings_.find(name);
    if (mapping_iter == control_mappings_.end()) return value;

    // Use the mapping to look up the control value.
    auto controls_iter = controls_by_name_.find(mapping_iter->second);
    if (controls_iter == controls_by_name_.end()) return value;

    float injected = signal.Inject(controls_iter->second);
    return injected;
  }

  bool enable_injection_ = false;

  std::set<int> buttons_{};
  std::vector<int> buttons_as_ints_{};
  std::vector<std::string> buttons_as_strings_{};
  absl::flat_hash_map<std::string, std::string> control_mappings_{};
  absl::flat_hash_map<std::string, Signal> signals_by_name_{};
  absl::flat_hash_map<std::string, int> button_mappings_{};
  absl::flat_hash_map<std::string, Counter> counters_by_name_{};
  absl::flat_hash_map<std::string, float> controls_by_name_{};

  ProtoPort<Control> control_port_;
};

// Injects a signal named `name` of the time history of `value`.
#define SIGINJECT(name, value) ControlInjector::InjectSignal((name), (value))
#define SIGINJECT_OVERRIDE(name, value, low, high) \
  ControlInjector::InjectSignalOverride((name), (value), (low), (high))
#define SIGINJECT_TRIGGER(name) ControlInjector::InjectTrigger((name))

// Injects a counter signal named `name`.
#define SIGINJECT_COUNTER(name, value, low, high) \
  ControlInjector::InjectCounter((name), (value), (low), (high))
#define SIGINJECT_ENUM(name, value) ControlInjector::InjectEnum((name), (value))

#define XSTR(X) #X
#define STR(X) XSTR(X)
// Injects the time history of `value`, automatically assigning a name.
#define SIGINJECT_AUTO(value)    \
  ControlInjector::InjectSignal( \
      ("" __FILE__ ":" STR(__LINE__) "| " #value " |"), (value))

// Same as above, but wraps `value` to the range given by [`low`, `high`).
#define SIGINJECT_CLAMP(name, value, low, high) \
  ControlInjector::InjectSignalClamp((name), (value), (low), (high))
#define SIGINJECT_AUTO_CLAMP(value, low, high)                          \
  ControlInjector::InjectSignalClamp(                                   \
      ("" __FILE__ ":" STR(__LINE__) "| " #value " |"), (value), (low), \
      (high))

// Declares a local variable and plots its time history.
//
// Declares a local variable named `name` and assigns `value` to it. Injects the
// time history of `value`, assigning `name` to it.
#define SIGINJECT_ASSIGN(name, value) \
  name = ControlInjector::InjectSignal(("" #name), (value))
#define SIGINJECT_ASSIGN_CLAMP(name, value, low, high) \
  name = ControlInjector::InjectSignalClamp(("" #name), (value), (low), (high))

}  // namespace opendrop

#endif  // DEBUG_CONTROL_INJECTOR_H_
