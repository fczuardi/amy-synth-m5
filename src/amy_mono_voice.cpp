#include "amy_mono_voice.h"

#include <AMY-Arduino.h>

void AmyMonoVoice::begin(uint8_t synthId,
                         uint8_t voiceCount,
                         uint8_t initialPatch) {
  synthId_ = synthId;
  voiceCount_ = voiceCount;
  patchNumber_ = initialPatch;
  activeMidiNote_ = 0;
  noteActive_ = false;
  begun_ = true;

  setPatch(patchNumber_);
}

void AmyMonoVoice::setPatch(uint8_t patchNumber) {
  patchNumber_ = patchNumber;
  if (!begun_) {
    return;
  }

  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.patch_number = patchNumber_;
  event.num_voices = voiceCount_;
  amy_add_event(&event);
}

void AmyMonoVoice::noteOn(uint8_t midiNote, float velocity) {
  if (!begun_) {
    return;
  }

  if (noteActive_) {
    noteOff();
  }

  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.midi_note = midiNote;
  event.velocity = velocity;
  amy_add_event(&event);

  activeMidiNote_ = midiNote;
  noteActive_ = true;
}

void AmyMonoVoice::noteOff() {
  if (!begun_ || !noteActive_) {
    return;
  }

  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.midi_note = activeMidiNote_;
  event.velocity = 0.0f;
  amy_add_event(&event);

  noteActive_ = false;
}

uint8_t AmyMonoVoice::synthId() const {
  return synthId_;
}

uint8_t AmyMonoVoice::voiceCount() const {
  return voiceCount_;
}

uint8_t AmyMonoVoice::patchNumber() const {
  return patchNumber_;
}

uint8_t AmyMonoVoice::activeMidiNote() const {
  return activeMidiNote_;
}

bool AmyMonoVoice::noteActive() const {
  return noteActive_;
}
