#pragma once

#include <cstddef>
#include <cstdint>

constexpr size_t AMY_M5_MIDI_CHANNEL_COUNT = 16;

struct AmyM5MonophonicSynthConfiguration {
  uint8_t synthId = 1;
  uint8_t voiceCount = 1;
  uint16_t patches[AMY_M5_MIDI_CHANNEL_COUNT] = {};
};

inline bool amyM5PatchForMidiChannel(
    const AmyM5MonophonicSynthConfiguration& configuration,
    uint8_t midiChannel,
    uint16_t& patchNumber) {
  if (midiChannel >= AMY_M5_MIDI_CHANNEL_COUNT) {
    return false;
  }

  patchNumber = configuration.patches[midiChannel];
  return true;
}
