#include "AmyMonophonicInstrumentSink.h"

AmyMonophonicInstrumentSink::AmyMonophonicInstrumentSink(
    AmyRuntime& runtime,
    AmySynthSlot& synthSlot,
    AmyAudioActivityGate& audioGate)
    : runtime_(runtime),
      synthSlot_(synthSlot),
      audioGate_(audioGate) {
}

void AmyMonophonicInstrumentSink::onNoteEvent(const NoteEvent& event) {
  const bool hadActiveNote = notePriority_.isNoteActive();
  const uint8_t previousNote = notePriority_.activeMidiNote();
  const MonophonicNoteAction action = notePriority_.handleNoteEvent(event);

  if (action.type == MonophonicNoteActionType::StartNote && hadActiveNote) {
    synthSlot_.noteOff(previousNote);
  }

  if (noteActionObserver_ != nullptr &&
      action.type == MonophonicNoteActionType::StartNote) {
    noteActionObserver_(noteActionObserverContext_, action);
  }

  applyNoteAction(action);
}

void AmyMonophonicInstrumentSink::setNoteActionObserver(
    NoteActionObserver observer,
    void* context) {
  noteActionObserver_ = observer;
  noteActionObserverContext_ = context;
}

void AmyMonophonicInstrumentSink::onPitchBendEvent(const PitchBendEvent& event) {
  runtime_.setGlobalPitchBend(event.value);
  pitchBend_ = event.value;
  audioGate_.wake();
}

void AmyMonophonicInstrumentSink::onDisconnected() {
  panic();
}

void AmyMonophonicInstrumentSink::panic() {
  runtime_.setGlobalPitchBend(0);
  pitchBend_ = 0;
  applyNoteAction(notePriority_.stopAll());
  audioGate_.wake();
}

bool AmyMonophonicInstrumentSink::noteActive() const {
  return notePriority_.isNoteActive();
}

uint8_t AmyMonophonicInstrumentSink::activeMidiChannel() const {
  return notePriority_.activeMidiChannel();
}

uint8_t AmyMonophonicInstrumentSink::activeMidiNote() const {
  return notePriority_.activeMidiNote();
}

int16_t AmyMonophonicInstrumentSink::pitchBend() const {
  return pitchBend_;
}

float AmyMonophonicInstrumentSink::normalizedVelocity(uint8_t velocity) {
  return static_cast<float>(velocity) / 127.0f;
}

void AmyMonophonicInstrumentSink::applyNoteAction(
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
