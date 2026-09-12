#include "AmyRuntime.h"

#include <cmath>

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
  lastSentPitchBend_ = globalPitchBend_;
}

void AmyRuntime::setGlobalModulation(uint8_t value) {
  globalModulation_ = value > 127 ? 127 : value;
}

void AmyRuntime::update(bool noteActive) {
  if (!begun_) {
    return;
  }

  const uint32_t nowMs = millis();
  if (!noteActive || globalModulation_ == 0) {
    if (lastSentPitchBend_ != globalPitchBend_) {
      setGlobalPitchBend(globalPitchBend_);
    }
    return;
  }

  // Keep the modulation source deliberately small and slow for the first
  // hardware experiment: roughly 5 Hz and at most a quarter semitone.
  if (nowMs - lastVibratoUpdateMs_ < 20) {
    return;
  }
  lastVibratoUpdateMs_ = nowMs;

  constexpr float VIBRATO_TWO_PI = 6.28318530718f;
  constexpr int16_t MAX_VIBRATO_BEND = 1024;
  const float phase =
      static_cast<float>(nowMs % 200) * VIBRATO_TWO_PI / 200.0f;
  const int16_t offset = static_cast<int16_t>(
      sinf(phase) * MAX_VIBRATO_BEND * globalModulation_ / 127.0f);
  const int16_t bend = AmyPitchBend::clamp(
      static_cast<int16_t>(globalPitchBend_ + offset));
  if (bend == lastSentPitchBend_) {
    return;
  }

  amy_event event = amy_default_event();
  event.synth = controlSynthId_;
  event.note_source_channel = controlSynthId_;
  event.pitch_bend = AmyPitchBend::toOctaves(bend);
  amy_add_event(&event);
  lastSentPitchBend_ = bend;
}

int16_t AmyRuntime::globalPitchBend() const {
  return globalPitchBend_;
}

uint8_t AmyRuntime::globalModulation() const {
  return globalModulation_;
}

uint8_t AmyRuntime::controlSynthId() const {
  return controlSynthId_;
}
