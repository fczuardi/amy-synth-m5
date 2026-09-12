#include "AmyM5MonophonicSynth.h"

#include <cstring>

#include <AMY-Arduino.h>

#include "AmyMidiControlMappingFormatter.h"

namespace {
constexpr uint8_t JUNO_TONAL_OSCILLATORS[] = {2, 3, 4};

const uint8_t* junoTonalOscillators() {
  return JUNO_TONAL_OSCILLATORS;
}

size_t junoTonalOscillatorCount() {
  return sizeof(JUNO_TONAL_OSCILLATORS) /
         sizeof(JUNO_TONAL_OSCILLATORS[0]);
};
}  // namespace

AmyM5MonophonicSynth::AmyM5MonophonicSynth()
    : audioGate_(speakerBridge_),
      instrumentSink_(runtime_, synthSlot_, audioGate_) {
  instrumentSink_.setNoteActionObserver(
      [](void* context, const MonophonicNoteAction& action) {
        auto* synth = static_cast<AmyM5MonophonicSynth*>(context);
        if (!synth->supportsMidiChannel(action.midiChannel)) {
          return;
        }
        const uint8_t patch =
            synth->patchNumberForChannel(action.midiChannel);
        if (patch != synth->selectedPatch_) {
          synth->synthSlot_.setPatch(patch);
          synth->selectedPatch_ = patch;
        }
      },
      this);
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
  const uint32_t renderedBlocksBefore = speakerBridge_.renderedBlockCount();
  audioGate_.update(instrumentSink_.noteActive());

  if (controlsRestorePending_ &&
      speakerBridge_.renderedBlockCount() != renderedBlocksBefore) {
    restoreControlValues(controlsRestoreChannel_);
    controlsRestorePending_ = false;
  }
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

  size_t mappingIndex = 0;
  while (mappingIndex < controlValueCount_ &&
         controlValues_[mappingIndex].controller != mapping.controller) {
    ++mappingIndex;
  }
  if (mappingIndex == controlValueCount_ &&
      controlValueCount_ == MAX_MIDI_CONTROL_MAPPINGS) {
    return false;
  }

  return storeMidiControlMapping(mapping, message, length);
}

bool AmyM5MonophonicSynth::storeMidiControlMapping(
    const AmyMidiControlMapping& mapping,
    const char* message,
    size_t messageLength) {
  if (message == nullptr || messageLength == 0) {
    return false;
  }

  size_t mappingIndex = 0;
  while (mappingIndex < controlValueCount_ &&
         controlValues_[mappingIndex].controller != mapping.controller) {
    ++mappingIndex;
  }
  if (mappingIndex == controlValueCount_ &&
      controlValueCount_ == MAX_MIDI_CONTROL_MAPPINGS) {
    return false;
  }

  const bool stored = midi_store_mapping(
             static_cast<int>(mapping.midiChannel) + 1,
             MIDI_MAP_TYPE_CC,
             mapping.controller,
             0,
             mapping.coefficientAtMinimum,
             mapping.coefficientAtMaximum,
             0.0f,
             message,
             messageLength) != 0;
  if (stored) {
    if (mappingIndex == controlValueCount_) {
      controlValues_[mappingIndex].controller = mapping.controller;
      ++controlValueCount_;
    }
  }
  return stored;
}

bool AmyM5MonophonicSynth::configureJunoPerformanceModulation(
    uint8_t controller) {
  if (!begun_ || controller > 127) {
    return false;
  }

  bool configured = true;
  const uint8_t channels[] = {
      FIRST_MIDI_CHANNEL,
      SECOND_MIDI_CHANNEL,
  };
  const size_t channelCount = secondPatchEnabled_ ? 2 : 1;

  for (size_t channelIndex = 0; channelIndex < channelCount; ++channelIndex) {
    const AmyMidiControlMapping mapping{
        .midiChannel = channels[channelIndex],
        .controller = controller,
        .targetOscillator = junoTonalOscillators()[0],
        .source = AmyModulationSource::Mod0,
        .target = AmyModulationTarget::Frequency,
        .coefficientAtMinimum = 0.0f,
        .coefficientAtMaximum = 0.1f,
    };

    char message[128] = {};
    size_t messageLength = 0;
    for (size_t oscillatorIndex = 0;
         oscillatorIndex < junoTonalOscillatorCount();
         ++oscillatorIndex) {
      AmyMidiControlMapping oscillatorMapping = mapping;
      oscillatorMapping.targetOscillator =
          junoTonalOscillators()[oscillatorIndex];

      char oscillatorMessage[48] = {};
      size_t oscillatorMessageLength = 0;
      if (!formatAmyMidiControlMessage(
              synthSlot_.synthId(),
              oscillatorMapping,
              oscillatorMessage,
              sizeof(oscillatorMessage),
              oscillatorMessageLength) ||
          messageLength + oscillatorMessageLength >= sizeof(message)) {
        configured = false;
        break;
      }

      std::memcpy(
          message + messageLength,
          oscillatorMessage,
          oscillatorMessageLength);
      messageLength += oscillatorMessageLength;
    }

    configured = storeMidiControlMapping(mapping, message, messageLength) &&
                 configured;
  }

  return configured;
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

  instrumentSink_.onNoteEvent(event);

  if (instrumentSink_.noteActive()) {
    // A fallback may have reloaded the patch. Defer the replay until the
    // audio bridge has rendered a block, so AMY has completed that patch load.
    controlsRestoreChannel_ = instrumentSink_.activeMidiChannel();
    controlsRestorePending_ = true;
  } else {
    controlsRestorePending_ = false;
  }
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
  if (!supportsMidiChannel(event.channel)) {
    return;
  }

  rememberControlValue(event);

  if (!instrumentSink_.noteActive()) {
    return;
  }

  const uint8_t targetChannel = secondPatchEnabled_
      ? instrumentSink_.activeMidiChannel()
      : event.channel;
  sendControlChange({targetChannel, event.controller, event.value});
}

void AmyM5MonophonicSynth::rememberControlValue(
    const ControlChangeEvent& event) {
  for (size_t i = 0; i < controlValueCount_; ++i) {
    StoredControlValue& stored = controlValues_[i];
    if (stored.controller == event.controller) {
      stored.value = event.value;
      stored.hasValue = true;
      return;
    }
  }
}

void AmyM5MonophonicSynth::restoreControlValues(uint8_t midiChannel) {
  for (size_t i = 0; i < controlValueCount_; ++i) {
    const StoredControlValue& stored = controlValues_[i];
    if (stored.hasValue) {
      sendControlChange({
          midiChannel,
          stored.controller,
          stored.value,
      });
    }
  }
}

void AmyM5MonophonicSynth::sendControlChange(
    const ControlChangeEvent& event) {
  uint8_t rawMessage[3] = {
      static_cast<uint8_t>(0xB0 | event.channel),
      event.controller,
      event.value,
  };
  midi_msg_handler(rawMessage, sizeof(rawMessage), 0, amy_sysclock());
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
