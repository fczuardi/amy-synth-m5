#pragma once

#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyMonophonicInstrumentSink.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyMidiControlMapping.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"
#include "InstrumentEventSink.h"

// Convenient Core Gray AMY composition for one global monophonic instrument.
//
// The application still initializes M5Unified before calling begin(). This
// facade owns the AMY runtime, one synth slot, the M5 speaker bridge, the
// activity gate, and one monophonic event policy. In the optional two-patch
// mode, the MIDI channel selects the patch for the next note.
class AmyM5MonophonicSynth : public InstrumentEventSink {
 public:
  // MIDI event channels are preserved as zero-based status nibbles: channel
  // 0 is the physical MIDI channel 1, and channel 1 is physical channel 2.
  static constexpr uint8_t FIRST_MIDI_CHANNEL = 0;
  static constexpr uint8_t SECOND_MIDI_CHANNEL = 1;

  AmyM5MonophonicSynth();

  // Configures one patch for a global monophonic instrument.
  void begin(uint8_t synthId, uint8_t voiceCount, uint8_t initialPatch);

  // Configures two patches selected by the raw MIDI channels 0 and 1.
  void begin(
      uint8_t firstSynthId,
      uint8_t voiceCount,
    uint8_t firstPatch,
    uint8_t secondPatch);
  void update();

  // Registers a MIDI CC mapping using AMY's native mapping mechanism. The
  // MIDI channel is zero-based, like ControlChangeEvent; patch-specific
  // oscillator choices remain with the application configuration.
  bool configureMidiControlMapping(const AmyMidiControlMapping& mapping);

  void setPatch(uint8_t patchNumber);
  void panic();

  void onNoteEvent(const NoteEvent& event) override;
  void onPitchBendEvent(const PitchBendEvent& event) override;
  void onControlChangeEvent(const ControlChangeEvent& event) override;
  void onDisconnected() override;

  bool noteActive() const;
  uint8_t patchNumber() const;
  bool supportsMidiChannel(uint8_t midiChannel) const;
  uint8_t patchNumberForChannel(uint8_t midiChannel) const;
  int16_t pitchBend() const;

 private:
  static constexpr size_t MAX_MIDI_CONTROL_MAPPINGS = 8;

  struct StoredControlValue {
    uint8_t midiChannel = 0;
    uint8_t controller = 0;
    uint8_t value = 0;
    bool hasValue = false;
  };

  void rememberControlValue(const ControlChangeEvent& event);
  void restoreControlValues(uint8_t midiChannel);
  void sendControlChange(const ControlChangeEvent& event);

  AmyM5SpeakerBridge speakerBridge_;
  AmyAudioActivityGate audioGate_;
  AmyRuntime runtime_;
  AmySynthSlot synthSlot_;
  AmyMonophonicInstrumentSink instrumentSink_;
  uint8_t firstPatch_ = 0;
  uint8_t secondPatch_ = 0;
  uint8_t selectedPatch_ = 0;
  bool secondPatchEnabled_ = false;
  bool begun_ = false;
  StoredControlValue controlValues_[MAX_MIDI_CONTROL_MAPPINGS] = {};
  size_t controlValueCount_ = 0;
};
