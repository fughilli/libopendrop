#ifndef PRESET_PRESET_LIST_H_
#define PRESET_PRESET_LIST_H_

#include <memory>
#include <random>

#include "absl/status/statusor.h"
#include "preset/preset.h"
#include "util/logging/logging.h"
#include "util/status/status_macros.h"

// Preset includes
#include "preset/alien_rorschach/alien_rorschach.h"
#include "preset/cube_boom/cube_boom.h"
#include "preset/cube_wreath/cube_wreath.h"
#include "preset/eye_roll/eye_roll.h"
#include "preset/glowsticks_3d/glowsticks_3d.h"
#include "preset/glowsticks_3d_zoom/glowsticks_3d_zoom.h"
#include "preset/graph_preset/graph_preset.h"
#include "preset/kaleidoscope/kaleidoscope.h"
#include "preset/pills/pills.h"
#include "preset/rotary_transporter/rotary_transporter.h"
#include "preset/shape_bounce/shape_bounce.h"
#include "preset/simple_preset/simple_preset.h"
#include "preset/space_whale_eye_warp/space_whale_eye_warp.h"
#include "preset/template_preset/template_preset.h"

namespace opendrop {

namespace preset_list {
// Implementation detail for `GetRandomPreset`. This class is used to hint to
// the compiler how to separate `Args` from `Presets` in the implementation of
// `GetRandomPreset`.
template <typename... Args>
struct GetRandomPresetHelperStruct {
  // Base case.
  template <typename PresetA>
  static absl::StatusOr<std::shared_ptr<opendrop::Preset>>
  GetRandomPresetHelper(int index, Args&&... args) {
    if (index != 0) {
      return absl::FailedPreconditionError(
          "Index greater than number of presets");
    }

    return PresetA::MakeShared(std::forward<Args>(args)...);
  }

  // Recursively unpack the parameter pack until index is 0, and instatiate that
  // class.
  template <typename PresetA, typename PresetB, typename... Presets>
  static absl::StatusOr<std::shared_ptr<opendrop::Preset>>
  GetRandomPresetHelper(int index, Args&&... args) {
    if (index == 0) {
      return PresetA::MakeShared(std::forward<Args>(args)...);
    }

    return GetRandomPresetHelper<PresetB, Presets...>(
        index - 1, std::forward<Args>(args)...);
  }
};
}  // namespace preset_list

// Returns a shared pointer to an instance of a random `Preset` subclass. The
// subclass is chosen from the template parameter pack `Presets` at random. The
// instance is constructed by forwarding `args`.
template <typename... Presets, typename... Args>
absl::StatusOr<std::shared_ptr<opendrop::Preset>> GetRandomPreset(
    Args&&... args) {
  // Division modulus for the RNG
  static const int kModulus = sizeof...(Presets);

  // RNG configuration. Make it static so we get a random sequence over time.
  static std::random_device device;
  static std::default_random_engine random_engine(device());
  static std::uniform_int_distribution<int> distribution(0, kModulus - 1);

  // Pull a random index from the distribution. Shift it if it's the same index
  // we got last time.
  auto random_index = distribution(random_engine);
  static auto last_selection = 0;
  if (random_index == last_selection) {
    random_index = (last_selection + 1) % kModulus;
  }
  last_selection = random_index;

  ASSIGN_OR_RETURN(auto return_preset,
                   preset_list::GetRandomPresetHelperStruct<Args...>::
                       template GetRandomPresetHelper<Presets...>(
                           random_index, std::forward<Args>(args)...));

  LOG(DEBUG) << "Returning preset with index " << random_index << " and name "
             << return_preset->name();

  return return_preset;
}

template <typename... Args>
absl::StatusOr<std::shared_ptr<opendrop::Preset>> GetRandomPresetFromList(
    Args&&... args) {
  return GetRandomPreset<opendrop::GraphPreset>(std::forward<Args>(args)...);
}

}  // namespace opendrop

#endif  // PRESET_PRESET_LIST_H_
