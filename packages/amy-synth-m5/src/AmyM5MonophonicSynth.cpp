#include "AmyM5MonophonicSynth.h"

#include <AMY-Arduino.h>

#include "AmyMidiControlMappingFormatter.h"

AmyM5MonophonicSynth::AmyM5MonophonicSynth()
    : audioGate_(speakerBridge_),
      instrumentSink_(runtime_, synthSlot_, audioGate_) {
}

void AmyM5MonophonicSynth::begin(
    uint8_t synthId,
    uint8_t voiceCount,
    uint8_t initialPatch) {
  speakerBridge_.begin();
  runtime_.begin(synthId);
  synthSlot_.begin(synthId, voiceCount, initialPatch);
  firstPatch_ = initialPatch;
  secondPatch_ = initialPatch;
  selectedPatch_ = initialPatch;
  secondPatchEnabled_ = false;
  begun_ = true;
}

void AmyM5MonophonicSynth::begin(
    uint8_t firstSynthId,
    uint8_t voiceCount,
    uint8_t firstPatch,
    uint8_t secondPatch) {
  speakerBridge_.begin();
  runtime_.begin(firstSynthId);
  synthSlot_.begin(firstSynthId, voiceCount, firstPatch);
  firstPatch_ = firstPatch;
  secondPatch_ = secondPatch;
  selectedPatch_ = firstPatch;
  secondPatchEnabled_ = true;
  begun_ = true;
}

void AmyM5MonophonicSynth::update() {
  audioGate_.update(instrumentSink_.noteActive());
}

bool AmyM5MonophonicSynth::configureMidiControlMapping(
    const AmyMidiControlMapping& mapping) {
  if (mapping.midiChannel > 15 || mapping.controller > 127) {
    return false;
  }

  if (!begun_) {
    return false;
  }

  char message[64];
  size_t length = 0;
  if (!formatAmyMidiControlMessage(
          synthSlot_.synthId(), mapping, message, sizeof(message), length)) {
    return false;
  }

  return midi_store_mapping(
             static_cast<int>(mapping.midiChannel) + 1,
             MIDI_MAP_TYPE_CC,
             mapping.controller,
             0,
             mapping.coefficientAtMinimum,
             mapping.coefficientAtMaximum,
             0.0f,
             message,
             length) != 0;
}

void AmyM5MonophonicSynth::setPatch(uint8_t patchNumber) {
  synthSlot_.setPatch(patchNumber);
  firstPatch_ = patchNumber;
  selectedPatch_ = patchNumber;
  secondPatchEnabled_ = false;
}

void AmyM5MonophonicSynth::panic() {
  instrumentSink_.panic();
}

void AmyM5MonophonicSynth::onNoteEvent(const NoteEvent& event) {
  if (!supportsMidiChannel(event.channel)) {
    return;
  }

  if (event.type == NoteEventType::NoteOn) {
    const uint8_t patch = patchNumberForChannel(event.channel);
    if (patch != selectedPatch_) {
      synthSlot_.setPatch(patch);
      selectedPatch_ = patch;
    }
  }

  instrumentSink_.onNoteEvent(event);
}

void AmyM5MonophonicSynth::onPitchBendEvent(const PitchBendEvent& event) {
  if (secondPatchEnabled_ &&
      (!instrumentSink_.noteActive() ||
       event.channel != instrumentSink_.activeMidiChannel())) {
    return;
  }
  instrumentSink_.onPitchBendEvent(event);
}

void AmyM5MonophonicSynth::onControlChangeEvent(
    const ControlChangeEvent& event) {
  if (!supportsMidiChannel(event.channel) ||
      (secondPatchEnabled_ &&
       (!instrumentSink_.noteActive() ||
        event.channel != instrumentSink_.activeMidiChannel()))) {
    return;
  }

  uint8_t rawMessage[3] = {
      static_cast<uint8_t>(0xB0 | event.channel),
      event.controller,
      event.value,
  };
  midi_msg_handler(rawMessage, sizeof(rawMessage), 0, 0);
}

void AmyM5MonophonicSynth::onDisconnected() {
  instrumentSink_.onDisconnected();
}

bool AmyM5MonophonicSynth::noteActive() const {
  return instrumentSink_.noteActive();
}

uint8_t AmyM5MonophonicSynth::patchNumber() const {
  return selectedPatch_;
}

bool AmyM5MonophonicSynth::supportsMidiChannel(uint8_t midiChannel) const {
  return !secondPatchEnabled_ || midiChannel == FIRST_MIDI_CHANNEL ||
         midiChannel == SECOND_MIDI_CHANNEL;
}

uint8_t AmyM5MonophonicSynth::patchNumberForChannel(
    uint8_t midiChannel) const {
  if (!secondPatchEnabled_) {
    return firstPatch_;
  }
  if (midiChannel == FIRST_MIDI_CHANNEL) {
    return firstPatch_;
  }
  if (midiChannel == SECOND_MIDI_CHANNEL) {
    return secondPatch_;
  }
  return 0;
}

int16_t AmyM5MonophonicSynth::pitchBend() const {
  return instrumentSink_.pitchBend();
}
