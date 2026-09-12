#pragma once

#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyMonophonicInstrumentSink.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyM5MonophonicSynthConfiguration.h"
#include "AmyMidiControlMapping.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"
#include "InstrumentEventSink.h"

// Convenient Core Gray AMY composition for one global monophonic instrument.
//
// The application still initializes M5Unified before calling begin(). This
// facade owns the AMY runtime, one synth slot, the M5 speaker bridge, the
// activity gate, and one monophonic event policy. Every MIDI channel selects a
// patch from the copied fixed-size configuration; this is not multitimbrality.
class AmyM5MonophonicSynth : public InstrumentEventSink {
 public:
  AmyM5MonophonicSynth();

  // Copies the fixed-size configuration, so the caller may release it after
  // begin() returns.
  void begin(const AmyM5MonophonicSynthConfiguration& configuration);
  void update();

  // Registers a MIDI CC mapping using AMY's native mapping mechanism. The
  // MIDI channel is zero-based, like ControlChangeEvent; patch-specific
  // oscillator choices remain with the application configuration.
  bool configureMidiControlMapping(const AmyMidiControlMapping& mapping);

  // Applies a Juno-style performance mapping to the configured channels. It
  // maps the controller to the existing relative Juno LFO/mod0 route on each
  // tonal oscillator.
  bool configureJunoPerformanceModulation(uint8_t controller = 1);

  void setPatch(uint16_t patchNumber);
  void panic();

  void onNoteEvent(const NoteEvent& event) override;
  void onPitchBendEvent(const PitchBendEvent& event) override;
  void onControlChangeEvent(const ControlChangeEvent& event) override;
  void onDisconnected() override;

  bool noteActive() const;
  uint16_t patchNumber() const;
  bool supportsMidiChannel(uint8_t midiChannel) const;
  bool patchNumberForChannel(uint8_t midiChannel, uint16_t& patchNumber) const;
  int16_t pitchBend() const;

 private:
  static constexpr size_t MAX_MIDI_CONTROL_MAPPINGS = 8;

  struct StoredControlValue {
    uint8_t controller = 0;
    uint8_t value = 0;
    bool hasValue = false;
  };

  void rememberControlValue(const ControlChangeEvent& event);
  void restoreControlValues(uint8_t midiChannel);
  bool storeMidiControlMapping(
      const AmyMidiControlMapping& mapping,
      const char* message,
      size_t messageLength);
  void sendControlChange(const ControlChangeEvent& event);

  AmyM5SpeakerBridge speakerBridge_;
  AmyAudioActivityGate audioGate_;
  AmyRuntime runtime_;
  AmySynthSlot synthSlot_;
  AmyMonophonicInstrumentSink instrumentSink_;
  AmyM5MonophonicSynthConfiguration configuration_ = {};
  uint16_t selectedPatch_ = 0;
  bool begun_ = false;
  bool controlsRestorePending_ = false;
  uint8_t controlsRestoreChannel_ = 0;
  StoredControlValue controlValues_[MAX_MIDI_CONTROL_MAPPINGS] = {};
  size_t controlValueCount_ = 0;
};
