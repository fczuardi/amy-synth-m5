#include "AmySynthVoice.h"

#include <AMY-Arduino.h>

void AmySynthVoice::begin(uint8_t synthId,
                          uint8_t voiceCount,
                          uint8_t initialPatch) {
  synthId_ = synthId;
  voiceCount_ = voiceCount;
  patchNumber_ = initialPatch;
  activeMidiNote_ = 0;
  pitchBend_ = 0;
  noteActive_ = false;
  begun_ = true;

  // Keep begin() useful as a complete initialization point for simple sketches.
  setPatch(patchNumber_);
}

void AmySynthVoice::setPatch(uint8_t patchNumber) {
  patchNumber_ = patchNumber;
  if (!begun_) {
    return;
  }

  // AMY patch selection is an event too: the synth slot receives the patch
  // number and the maximum voice count it may allocate for that patch.
  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.patch_number = patchNumber_;
  event.num_voices = voiceCount_;
  amy_add_event(&event);
}

void AmySynthVoice::noteOn(uint8_t midiNote, float velocity) {
  if (!begun_) {
    return;
  }

  // For AMY's Juno-style patches, MIDI note plus non-zero velocity acts as the
  // note-on message for this synth slot.
  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.midi_note = midiNote;
  event.velocity = velocity;
  amy_add_event(&event);

  activeMidiNote_ = midiNote;
  noteActive_ = true;
}

void AmySynthVoice::noteOff(uint8_t midiNote) {
  if (!begun_) {
    return;
  }

  // A zero-velocity event releases the requested MIDI note.
  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.midi_note = midiNote;
  event.velocity = 0.0f;
  amy_add_event(&event);

  if (noteActive_ && activeMidiNote_ == midiNote) {
    noteActive_ = false;
  }
}

void AmySynthVoice::setPitchBend(int16_t value) {
  pitchBend_ = clampedPitchBend(value);
  if (!begun_) {
    return;
  }

  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.pitch_bend = pitchBendOctaves(pitchBend_);
  amy_add_event(&event);
}

void AmySynthVoice::stopActiveNote() {
  if (!noteActive_) {
    return;
  }

  noteOff(activeMidiNote_);
}

uint8_t AmySynthVoice::synthId() const {
  return synthId_;
}

uint8_t AmySynthVoice::voiceCount() const {
  return voiceCount_;
}

uint8_t AmySynthVoice::patchNumber() const {
  return patchNumber_;
}

uint8_t AmySynthVoice::activeMidiNote() const {
  return activeMidiNote_;
}

int16_t AmySynthVoice::pitchBend() const {
  return pitchBend_;
}

bool AmySynthVoice::noteActive() const {
  return noteActive_;
}

int16_t AmySynthVoice::clampedPitchBend(int16_t value) {
  if (value < -8192) {
    return -8192;
  }
  if (value > 8191) {
    return 8191;
  }
  return value;
}

float AmySynthVoice::pitchBendOctaves(int16_t value) {
  return static_cast<float>(value) / (6.0f * 8192.0f);
}
