#include <random>

#include "libopendrop/preset/kaleidoscope/kaleidoscope.h"
#include "libopendrop/preset/simple_preset/simple_preset.h"
#include "libopendrop/util/logging.h"

namespace opendrop {

template <typename... Args>
struct GetRandomPresetHelperClass {
  template <typename PresetA>
  static std::shared_ptr<opendrop::Preset> GetRandomPresetHelper(
      int index, Args&&... args) {
    CHECK(index == 0) << "Index greater than number of presets";

    return std::make_shared<PresetA>(std::forward<Args>(args)...);
  }

  template <typename PresetA, typename PresetB, typename... Presets>
  static std::shared_ptr<opendrop::Preset> GetRandomPresetHelper(
      int index, Args&&... args) {
    if (index == 0) {
      return std::make_shared<PresetA, Args...>(std::forward<Args>(args)...);
    }

    return GetRandomPresetHelper<PresetB, Presets...>(
        index - 1, std::forward<Args>(args)...);
  }
};

template <typename... Presets, typename... Args>
std::shared_ptr<opendrop::Preset> GetRandomPreset(Args&&... args) {
  static std::random_device device;
  static std::default_random_engine random_engine(device());
  static std::uniform_int_distribution<int> distribution(
      0, sizeof...(Presets) - 1);

  return GetRandomPresetHelperClass<Args...>::template GetRandomPresetHelper<
      Presets...>(distribution(random_engine), std::forward<Args>(args)...);
}

}  // namespace opendrop
