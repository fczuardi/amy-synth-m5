#pragma once

#include <cstdint>

constexpr int16_t AMY_M5_PITCH_BEND_CENTER_DEAD_ZONE = 128;

inline int16_t amyM5NormalizePitchBendCenter(int16_t value) {
  return value >= -AMY_M5_PITCH_BEND_CENTER_DEAD_ZONE &&
             value <= AMY_M5_PITCH_BEND_CENTER_DEAD_ZONE
      ? 0
      : value;
}

inline bool amyM5ShouldApplyPitchBend(
    bool noteActive,
    uint8_t activeMidiChannel,
    uint8_t eventMidiChannel,
    int16_t eventValue) {
  if (!noteActive) {
    return amyM5NormalizePitchBendCenter(eventValue) == 0;
  }

  return eventMidiChannel == activeMidiChannel;
}
