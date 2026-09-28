#pragma once

#include <stdint.h>

#include "AmyAudioActivityGate.h"
#include "AmyM5SpeakerBridge.h"
#include "AmySynthSlot.h"
#include "AmyTriggerOutput.h"
#include "TriggerEventSink.h"

struct AmyM5TriggerOutputConfig {
  uint8_t synthId = 1;
  uint8_t voiceCount = 1;
  uint16_t patch = 258;
  uint8_t speakerVolume = 128;
};

// Complete M5 speaker output for a fixed-lane trigger composition.
//
// The application still owns M5.begin() because display, buttons, microphone,
// and other board-wide choices are outside the audio backend.
class AmyM5TriggerOutput final : public TriggerEventSink {
 public:
  explicit AmyM5TriggerOutput(
      uint32_t tailMs = AmyTriggerOutput::DEFAULT_TAIL_MS)
      : gate_(bridge_), sink_(gate_, slot_, tailMs) {}

  void begin(const AmyM5TriggerOutputConfig& config = {});
  bool configureLane(uint8_t lane, const AmyTriggerVoice& voice);
  void clearLane(uint8_t lane);

  void trigger(const TriggerEvent& event) override;
  void update();
  void forceIdle();

 private:
  AmyM5SpeakerBridge bridge_;
  AmyAudioActivityGate gate_;
  AmySynthSlot slot_;
  AmyTriggerSink sink_;
};
