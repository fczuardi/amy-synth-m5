#pragma once

#include <Arduino.h>

// Owns AMY-wide musical controls that are not scoped to a single synth slot.
//
// AMY pitch bend behaves like shared runtime state, so callers should send it
// through this object instead of through an individual slot wrapper.
class AmyRuntime {
 public:
  void begin(uint8_t controlSynthId);

  void setGlobalPitchBend(int16_t value);

  int16_t globalPitchBend() const;
  uint8_t controlSynthId() const;

 private:
  int16_t globalPitchBend_ = 0;
  uint8_t controlSynthId_ = 0;
  bool begun_ = false;
};
