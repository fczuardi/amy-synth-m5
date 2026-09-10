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

  // Mark the output path as needed. Apps call this before musical events that
  // can produce sound, including note-on, note-off release tails, bend changes,
  // and panic/cleanup events.
  void wake(uint32_t tailMs = DEFAULT_RELEASE_TAIL_MS);

  // Drive the bridge while a note is active or while the release tail is still
  // open. Returns whether the audio output path is currently awake.
  bool update(bool soundActive);

  // Immediately close the output path. Useful for explicit mute/panic controls.
  void forceIdle();
  bool awake() const;

 private:
  AmyM5SpeakerBridge& bridge_;
  uint32_t awakeUntilMs_ = 0;
  bool awake_ = false;
};
