#pragma once

#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"
#include "InstrumentEventSink.h"
#include "MonophonicNotePriority.h"

// Applies shared instrument events to a monophonic AMY synth slot.
//
// This boundary is intentionally above AmyRuntime and AmySynthSlot: it owns the
// note priority policy, wakes the audio gate on musical activity, and leaves
// patch selection plus raw AMY event translation to the lower-level objects.
class AmyMonophonicInstrumentSink : public InstrumentEventSink {
 public:
  AmyMonophonicInstrumentSink(
      AmyRuntime& runtime,
      AmySynthSlot& synthSlot,
      AmyAudioActivityGate& audioGate);

  void onNoteEvent(const NoteEvent& event) override;
  void onPitchBendEvent(const PitchBendEvent& event) override;
  void onDisconnected() override;

  void panic();

  bool noteActive() const;
  uint8_t activeMidiChannel() const;
  uint8_t activeMidiNote() const;
  int16_t pitchBend() const;

 private:
  static float normalizedVelocity(uint8_t velocity);
  void applyNoteAction(const MonophonicNoteAction& action);

  AmyRuntime& runtime_;
  AmySynthSlot& synthSlot_;
  AmyAudioActivityGate& audioGate_;
  MonophonicNotePriority notePriority_;

  int16_t pitchBend_ = 0;
};
