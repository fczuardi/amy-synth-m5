#include "AmyAudioActivityGate.h"

AmyAudioActivityGate::AmyAudioActivityGate(AmyM5SpeakerBridge& bridge)
    : bridge_(bridge) {}

void AmyAudioActivityGate::wake(uint32_t tailMs) {
  if (!awake_) {
    bridge_.resumeOutput();
  }

  awakeUntilMs_ = millis() + tailMs;
  awake_ = true;
}

bool AmyAudioActivityGate::update(bool soundActive) {
  const bool withinReleaseTail =
      static_cast<int32_t>(millis() - awakeUntilMs_) < 0;

  if (soundActive || withinReleaseTail) {
    bridge_.update();
    awake_ = true;
    return true;
  }

  if (awake_) {
    forceIdle();
  }

  return false;
}

void AmyAudioActivityGate::forceIdle() {
  bridge_.stopOutput();
  awake_ = false;
}

bool AmyAudioActivityGate::awake() const {
  return awake_;
}
