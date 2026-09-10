#pragma once

#include <Arduino.h>

#include "AmyM5SpeakerBridge.h"

// Small output lifecycle helper for Core Gray style audio paths.
//
// AMY can keep rendering silence indefinitely, but the Core Gray speaker/DAC
// path is audibly open while silent PCM is streamed. This gate lets apps render
// while a note is active, keep a short release tail, then stop the speaker
// channel at rest.
class AmyAudioActivityGate {
 public:
  static constexpr uint32_t DEFAULT_RELEASE_TAIL_MS = 700;

  explicit AmyAudioActivityGate(AmyM5SpeakerBridge& bridge);

  void wake(uint32_t tailMs = DEFAULT_RELEASE_TAIL_MS);
  bool update(bool soundActive);
  void forceIdle();
  bool awake() const;

 private:
  AmyM5SpeakerBridge& bridge_;
  uint32_t awakeUntilMs_ = 0;
  bool awake_ = false;
};
