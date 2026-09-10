#include "AmyRuntime.h"

#include <AMY-Arduino.h>

void AmyRuntime::begin(uint8_t controlSynthId) {
  controlSynthId_ = controlSynthId;
  begun_ = true;
}

void AmyRuntime::setGlobalPitchBend(int16_t value) {
  globalPitchBend_ = clampedPitchBend(value);

  amy_event event = amy_default_event();
  if (begun_) {
    // AMY applies pitch bend globally, but its MIDI path still tags the event
    // with the source channel/synth. Preserve that routing metadata here.
    event.synth = controlSynthId_;
    event.note_source_channel = controlSynthId_;
  }
  event.pitch_bend = pitchBendOctaves(globalPitchBend_);
  amy_add_event(&event);
}

int16_t AmyRuntime::globalPitchBend() const {
  return globalPitchBend_;
}

uint8_t AmyRuntime::controlSynthId() const {
  return controlSynthId_;
}

int16_t AmyRuntime::clampedPitchBend(int16_t value) {
  if (value < -8192) {
    return -8192;
  }
  if (value > 8191) {
    return 8191;
  }
  return value;
}

float AmyRuntime::pitchBendOctaves(int16_t value) {
  // Match AMY's own MIDI handler: -8192..8191 is roughly -2..+2 semitones.
  return static_cast<float>(value) / (6.0f * 8192.0f);
}
