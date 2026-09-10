#include "AmySynthSlot.h"

#include <AMY-Arduino.h>

void AmySynthSlot::begin(uint8_t synthId,
                         uint8_t voiceCount,
                         uint8_t initialPatch) {
  synthId_ = synthId;
  voiceCount_ = voiceCount;
  patchNumber_ = initialPatch;
  begun_ = true;

  // Keep begin() useful as a complete initialization point for simple sketches.
  setPatch(patchNumber_);
}

void AmySynthSlot::setPatch(uint8_t patchNumber) {
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

void AmySynthSlot::noteOn(uint8_t midiNote, float velocity) {
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
}

void AmySynthSlot::noteOff(uint8_t midiNote) {
  if (!begun_) {
    return;
  }

  // A zero-velocity event releases the requested MIDI note.
  amy_event event = amy_default_event();
  event.synth = synthId_;
  event.midi_note = midiNote;
  event.velocity = 0.0f;
  amy_add_event(&event);
}

uint8_t AmySynthSlot::synthId() const {
  return synthId_;
}

uint8_t AmySynthSlot::voiceCount() const {
  return voiceCount_;
}

uint8_t AmySynthSlot::patchNumber() const {
  return patchNumber_;
}
