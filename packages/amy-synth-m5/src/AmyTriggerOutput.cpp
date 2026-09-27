#include "AmyTriggerOutput.h"

void AmyTriggerOutput::trigger(uint8_t midiNote, StepLevel level,
                               const TriggerVelocityCurve& velocities) {
  if (level == StepLevel::Off) return;
  gate_.wake(tailMs_);
  slot_.noteOn(midiNote, velocities.velocityFor(level));
}

bool AmyTriggerSink::configureLane(uint8_t lane,
                                   const AmyTriggerVoice& voice) {
  if (lane >= LANE_COUNT) return false;
  voices_[lane] = voice;
  return true;
}

void AmyTriggerSink::clearLane(uint8_t lane) {
  if (lane < LANE_COUNT) voices_[lane] = {};
}

void AmyTriggerSink::trigger(const TriggerEvent& event) {
  if (event.lane >= LANE_COUNT) return;
  const AmyTriggerVoice& voice = voices_[event.lane];
  if (!voice.enabled) return;
  output_.trigger(voice.midiNote, event.level, voice.velocities);
}

