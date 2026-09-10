#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"

AmyM5SpeakerBridge bridge;
AmyAudioActivityGate gate(bridge);
AmyRuntime runtime;
AmySynthSlot slot;

void setup() {
  bridge.begin();
  runtime.begin(1);
  slot.begin(1, 1, 19);
  runtime.setGlobalPitchBend(0);
  slot.noteOn(72, 0.8f);
  slot.noteOff(72);
  gate.wake();
}

void loop() {
  gate.update(false);
}
