#include "AmyM5TriggerOutput.h"

#include <M5Unified.h>

void AmyM5TriggerOutput::begin(const AmyM5TriggerOutputConfig& config) {
  bridge_.begin();
  M5.Speaker.setVolume(config.speakerVolume);
  slot_.begin(config.synthId, config.voiceCount, config.patch);
}

bool AmyM5TriggerOutput::configureLane(uint8_t lane,
                                       const AmyTriggerVoice& voice) {
  return sink_.configureLane(lane, voice);
}

void AmyM5TriggerOutput::clearLane(uint8_t lane) {
  sink_.clearLane(lane);
}

void AmyM5TriggerOutput::trigger(const TriggerEvent& event) {
  sink_.trigger(event);
}

void AmyM5TriggerOutput::update() {
  gate_.update(false);
}

void AmyM5TriggerOutput::forceIdle() {
  gate_.forceIdle();
}
