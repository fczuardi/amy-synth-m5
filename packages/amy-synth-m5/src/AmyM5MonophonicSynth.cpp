#include "AmyM5MonophonicSynth.h"

#include <cstring>

#include <AMY-Arduino.h>

#include "AmyM5PitchBendPolicy.h"
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
        uint16_t patch = 0;
        if (!synth->patchNumberForChannel(action.midiChannel, patch)) {
          return;
        }
        if (patch != synth->selectedPatch_) {
          synth->synthSlot_.setPatch(patch);
          synth->selectedPatch_ = patch;
        }
      },
      this);
}

void AmyM5MonophonicSynth::begin(
    const AmyM5MonophonicSynthConfiguration& configuration) {
  speakerBridge_.begin();
  configuration_ = configuration;
  runtime_.begin(configuration_.synthId);
  synthSlot_.begin(
      configuration_.synthId,
      configuration_.voiceCount,
      configuration_.patches[0]);
  selectedPatch_ = configuration_.patches[0];
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
  for (uint8_t channel = 0;
       channel < AMY_M5_MIDI_CHANNEL_COUNT;
       ++channel) {
    const AmyMidiControlMapping mapping{
        .midiChannel = channel,
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

void AmyM5MonophonicSynth::panic() {
  controlsRestorePending_ = false;
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
  const int16_t normalizedValue = amyM5NormalizePitchBendCenter(event.value);
  if (!supportsMidiChannel(event.channel) ||
      !amyM5ShouldApplyPitchBend(
          instrumentSink_.noteActive(),
          instrumentSink_.activeMidiChannel(),
          event.channel,
          normalizedValue)) {
    return;
  }
  instrumentSink_.onPitchBendEvent({event.channel, normalizedValue});
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

  const uint8_t targetChannel = instrumentSink_.activeMidiChannel();
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
  controlsRestorePending_ = false;
  instrumentSink_.onDisconnected();
}

bool AmyM5MonophonicSynth::noteActive() const {
  return instrumentSink_.noteActive();
}

uint16_t AmyM5MonophonicSynth::patchNumber() const {
  return selectedPatch_;
}

bool AmyM5MonophonicSynth::supportsMidiChannel(uint8_t midiChannel) const {
  return midiChannel < AMY_M5_MIDI_CHANNEL_COUNT;
}

bool AmyM5MonophonicSynth::patchNumberForChannel(
    uint8_t midiChannel,
    uint16_t& patchNumber) const {
  return amyM5PatchForMidiChannel(configuration_, midiChannel, patchNumber);
}

int16_t AmyM5MonophonicSynth::pitchBend() const {
  return instrumentSink_.pitchBend();
}
