#ifndef PRESET_PRESET_LIST_H_
#define PRESET_PRESET_LIST_H_

#include <memory>

#include "libopendrop/preset/preset.h"

namespace opendrop {

template <typename... Presets, typename... Args>
std::shared_ptr<opendrop::Preset> GetRandomPreset(Args&&... args);

template <typename... Args>
std::shared_ptr<opendrop::Preset> GetRandomPresetFromList(Args&&... args) {
  return GetRandomPreset<opendrop::Kaleidoscope, opendrop::SimplePreset,
                         opendrop::AlienRorschach>(std::forward<Args>(args)...);
}

}  // namespace opendrop

#endif  // PRESET_PRESET_LIST_H_
