#pragma once

#include <Arduino.h>

// Owns AMY events for one synth slot.
//
// This class deliberately does not know about M5Unified, BLE MIDI, buttons,
// display refresh, note replacement policy, or active-note bookkeeping. It
// only translates patch and note commands into AMY events for one slot.
class AmySynthSlot {
 public:
  // Configures the AMY synth slot and immediately applies the initial patch.
  void begin(uint8_t synthId, uint8_t voiceCount, uint8_t initialPatch);

  // Selects one of AMY's patch numbers for this synth slot.
  void setPatch(uint8_t patchNumber);

  // Sends a note-on event for this synth slot.
  void noteOn(uint8_t midiNote, float velocity);

  // Sends note-off for the requested MIDI note.
  void noteOff(uint8_t midiNote);

  uint8_t synthId() const;
  uint8_t voiceCount() const;
  uint8_t patchNumber() const;

 private:
  uint8_t synthId_ = 0;
  uint8_t voiceCount_ = 1;
  uint8_t patchNumber_ = 0;
  bool begun_ = false;
};
