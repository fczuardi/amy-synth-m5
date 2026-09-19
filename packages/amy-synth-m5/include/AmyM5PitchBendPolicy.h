#pragma once

#include <cstdint>

inline bool amyM5ShouldApplyPitchBend(
    bool noteActive,
    uint8_t activeMidiChannel,
    uint8_t eventMidiChannel) {
  return noteActive && eventMidiChannel == activeMidiChannel;
}
