#include "AmyPerformanceAdapter.h"

AmyPerformanceAdapter::AmyPerformanceAdapter(
    AmyRuntime& runtime,
    AmySynthSlot& synthSlot,
    AmyAudioActivityGate& audioGate)
    : runtime_(runtime),
      synthSlot_(synthSlot),
      audioGate_(audioGate) {
}

void AmyPerformanceAdapter::onNoteEvent(const NoteEvent& event) {
  const bool hadActiveNote = notePriority_.isNoteActive();
  const uint8_t previousNote = notePriority_.activeMidiNote();
  const MonophonicNoteAction action = notePriority_.handleNoteEvent(event);

  if (action.type == MonophonicNoteActionType::StartNote && hadActiveNote) {
    synthSlot_.noteOff(previousNote);
  }

  applyNoteAction(action);
}

void AmyPerformanceAdapter::onPitchBendEvent(const PitchBendEvent& event) {
  runtime_.setGlobalPitchBend(event.value);
  pitchBend_ = event.value;
  audioGate_.wake();
}

void AmyPerformanceAdapter::onDisconnected() {
  panic();
}

void AmyPerformanceAdapter::panic() {
  runtime_.setGlobalPitchBend(0);
  pitchBend_ = 0;
  applyNoteAction(notePriority_.stopAll());
  audioGate_.wake();
}

bool AmyPerformanceAdapter::noteActive() const {
  return notePriority_.isNoteActive();
}

uint8_t AmyPerformanceAdapter::activeMidiNote() const {
  return notePriority_.activeMidiNote();
}

int16_t AmyPerformanceAdapter::pitchBend() const {
  return pitchBend_;
}

float AmyPerformanceAdapter::normalizedVelocity(uint8_t velocity) {
  return static_cast<float>(velocity) / 127.0f;
}

void AmyPerformanceAdapter::applyNoteAction(
    const MonophonicNoteAction& action) {
  switch (action.type) {
    case MonophonicNoteActionType::None:
      return;
    case MonophonicNoteActionType::StartNote:
      audioGate_.wake();
      synthSlot_.noteOn(action.midiNote, normalizedVelocity(action.velocity));
      return;
    case MonophonicNoteActionType::StopNote:
      audioGate_.wake();
      synthSlot_.noteOff(action.midiNote);
      return;
  }
}
