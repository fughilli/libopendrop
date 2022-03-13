#ifndef DEBUG_CONTROL_INJECTOR_H_
#define DEBUG_CONTROL_INJECTOR_H_

#include <fstream>
#include <limits>
#include <set>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "debug/control.pb.h"
#include "debug/control_state.pb.h"
#include "debug/proto_port.h"
#include "google/protobuf/text_format.h"
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
    return static_cast<T>(
        instance().InjectSignalInternal(name, value, low, high));
  }

  static void SetStatePath(std::string path) { instance().state_path_ = path; }

  static void Save() {
    if (instance().state_path_ == "") return;
    std::ofstream output_proto(instance().state_path_.c_str());
    if (!output_proto.good()) return;
    std::string formatted;
    google::protobuf::TextFormat::PrintToString(instance().SaveToProto(),
                                                &formatted);
    output_proto << formatted;
  }

  static void Load() {
    if (instance().state_path_ == "") return;
    std::ifstream input_proto(instance().state_path_.c_str());
    if (!input_proto.good()) return;

    proto::ControlState control_state;
    std::stringstream buffer;
    buffer << input_proto.rdbuf();
    google::protobuf::TextFormat::ParseFromString(buffer.str(), &control_state);
    instance().LoadFromProto(control_state);
  }

 private:
  static constexpr size_t kDefaultHistorySize = 1024;

  struct Signal {
    float low = std::numeric_limits<float>::max();
    float high = std::numeric_limits<float>::lowest();

    void UpdateLimits(float value) {
      if (value < low) low = value;
      if (value > high) high = value;
    }

    void UpdateLimits(float low, float high) {
      this->low = low;
      this->high = high;
    }

    float Inject(float control) { return Lerp(low, high, control); }
  };

  struct Counter {
    int low;
    int high;

    int value;
    bool triggered;

    void Increment() {
      triggered = true;
      ++value;

      if (value > high) {
        value = low;
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

  // Saving/restoring config.
  proto::ControlState SaveToProto() {
    proto::ControlState control_state{};

    // Populate buttons_.
    for (int button : buttons_) control_state.add_buttons(button);

    // Populate mappings.
    for (auto& [signal_name, control_name] : control_mappings_)
      (*control_state.mutable_control_mappings())[signal_name] = control_name;

    for (auto& [signal_name, signal] : signals_by_name_) {
      proto::Signal proto_signal{};
      proto_signal.set_low(signal.low);
      proto_signal.set_high(signal.high);
      (*control_state.mutable_signals_by_name())[signal_name] = proto_signal;
    }

    for (auto& [counter_name, button] : button_mappings_)
      (*control_state.mutable_button_mappings())[counter_name] = button;

    for (auto& [counter_name, counter] : counters_by_name_) {
      proto::Counter proto_counter{};
      proto_counter.set_low(counter.low);
      proto_counter.set_high(counter.high);
      proto_counter.set_value(counter.value);
      (*control_state.mutable_counters_by_name())[counter_name] = proto_counter;
    }

    for (auto& [control_name, control_value] : controls_by_name_)
      (*control_state.mutable_controls_by_name())[control_name] = control_value;

    return control_state;
  }

  void LoadFromProto(const proto::ControlState& control_state) {
    // Populate buttons_.
    buttons_.clear();
    for (int button : control_state.buttons()) buttons_.insert(button);

    buttons_as_strings_.clear();
    buttons_as_ints_.clear();
    for (int button : buttons_) {
      buttons_as_ints_.push_back(button);
      buttons_as_strings_.push_back(absl::StrFormat("%d", button));
    }

    // Populate mappings.
    control_mappings_.clear();
    for (auto& [signal_name, control_name] : control_state.control_mappings())
      control_mappings_[signal_name] = control_name;

    signals_by_name_.clear();
    for (auto& [signal_name, signal] : control_state.signals_by_name())
      signals_by_name_[signal_name] = {.low = signal.low(),
                                       .high = signal.high()};

    button_mappings_.clear();
    for (auto& [counter_name, button] : control_state.button_mappings())
      button_mappings_[counter_name] = button;

    counters_by_name_.clear();
    for (auto& [counter_name, counter] : control_state.counters_by_name())
      counters_by_name_[counter_name] = {.low = counter.low(),
                                         .high = counter.high(),
                                         .value = counter.value()};

    controls_by_name_.clear();
    for (auto& [control_name, control_value] : control_state.controls_by_name())
      controls_by_name_[control_name] = control_value;
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
    if (ImGui::Button("Save")) Save();

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
          name, Counter{.low = low, .high = high, .value = value});
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

  std::string state_path_ = "";

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
