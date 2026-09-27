#pragma once

#include <stdint.h>

#include "AmyAudioActivityGate.h"
#include "AmySynthSlot.h"
#include "TriggerVelocityCurve.h"
#include "TriggerEventSink.h"

struct AmyTriggerVoice {
  uint8_t midiNote = 0;
  TriggerVelocityCurve velocities{};
  bool enabled = false;
};

class AmyTriggerOutput {
 public:
  static constexpr uint32_t DEFAULT_TAIL_MS = 1500;

  AmyTriggerOutput(AmyAudioActivityGate& gate, AmySynthSlot& slot,
                   uint32_t tailMs = DEFAULT_TAIL_MS)
      : gate_(gate), slot_(slot), tailMs_(tailMs) {}

  void trigger(uint8_t midiNote, StepLevel level,
               const TriggerVelocityCurve& velocities = {});

 private:
  AmyAudioActivityGate& gate_;
  AmySynthSlot& slot_;
  uint32_t tailMs_;
};

class AmyTriggerSink final : public TriggerEventSink {
 public:
  static constexpr uint8_t LANE_COUNT = 4;

  AmyTriggerSink(AmyAudioActivityGate& gate, AmySynthSlot& slot,
                 uint32_t tailMs = AmyTriggerOutput::DEFAULT_TAIL_MS)
      : output_(gate, slot, tailMs) {}

  bool configureLane(uint8_t lane, const AmyTriggerVoice& voice);
  void clearLane(uint8_t lane);
  void trigger(const TriggerEvent& event) override;

 private:
  AmyTriggerOutput output_;
  AmyTriggerVoice voices_[LANE_COUNT]{};
};
