#include "AmyM5MonophonicSynth.h"

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
}

void AmyM5MonophonicSynth::update() {
  audioGate_.update(instrumentSink_.noteActive());
}

void AmyM5MonophonicSynth::setPatch(uint8_t patchNumber) {
  synthSlot_.setPatch(patchNumber);
}

void AmyM5MonophonicSynth::panic() {
  instrumentSink_.panic();
}

void AmyM5MonophonicSynth::onNoteEvent(const NoteEvent& event) {
  instrumentSink_.onNoteEvent(event);
}

void AmyM5MonophonicSynth::onPitchBendEvent(const PitchBendEvent& event) {
  instrumentSink_.onPitchBendEvent(event);
}

void AmyM5MonophonicSynth::onDisconnected() {
  instrumentSink_.onDisconnected();
}

bool AmyM5MonophonicSynth::noteActive() const {
  return instrumentSink_.noteActive();
}

uint8_t AmyM5MonophonicSynth::patchNumber() const {
  return synthSlot_.patchNumber();
}

int16_t AmyM5MonophonicSynth::pitchBend() const {
  return instrumentSink_.pitchBend();
}
