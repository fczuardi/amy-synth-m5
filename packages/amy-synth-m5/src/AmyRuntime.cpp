#include "AmyRuntime.h"

#include <AMY-Arduino.h>

#include "AmyPitchBend.h"

void AmyRuntime::begin(uint8_t controlSynthId) {
  controlSynthId_ = controlSynthId;
  begun_ = true;
}

void AmyRuntime::setGlobalPitchBend(int16_t value) {
  globalPitchBend_ = AmyPitchBend::clamp(value);
  if (!begun_) {
    return;
  }

  amy_event event = amy_default_event();
  // AMY applies pitch bend globally, but its MIDI path still tags the event
  // with the source channel/synth. Preserve that routing metadata here.
  event.synth = controlSynthId_;
  event.note_source_channel = controlSynthId_;
  event.pitch_bend = AmyPitchBend::toOctaves(globalPitchBend_);
  amy_add_event(&event);
}

int16_t AmyRuntime::globalPitchBend() const {
  return globalPitchBend_;
}

uint8_t AmyRuntime::controlSynthId() const {
  return controlSynthId_;
}
