#pragma once

#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyMonophonicInstrumentSink.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"
#include "InstrumentEventSink.h"

// Convenient Core Gray AMY composition for the common monophonic case.
//
// The application still initializes M5Unified before calling begin(). This
// facade owns the AMY runtime, one synth slot, the M5 speaker bridge, the
// activity gate, and the monophonic event policy as one usable instrument.
class AmyM5MonophonicSynth : public InstrumentEventSink {
 public:
  AmyM5MonophonicSynth();

  void begin(uint8_t synthId, uint8_t voiceCount, uint8_t initialPatch);
  void update();

  void setPatch(uint8_t patchNumber);
  void panic();

  void onNoteEvent(const NoteEvent& event) override;
  void onPitchBendEvent(const PitchBendEvent& event) override;
  void onDisconnected() override;

  bool noteActive() const;
  uint8_t patchNumber() const;
  int16_t pitchBend() const;

 private:
  AmyM5SpeakerBridge speakerBridge_;
  AmyAudioActivityGate audioGate_;
  AmyRuntime runtime_;
  AmySynthSlot synthSlot_;
  AmyMonophonicInstrumentSink instrumentSink_;
};
