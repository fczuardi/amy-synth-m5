#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyPerformanceAdapter.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"

AmyM5SpeakerBridge bridge;
AmyAudioActivityGate gate(bridge);
AmyRuntime runtime;
AmySynthSlot slot;
AmyPerformanceAdapter performance(runtime, slot, gate);

void setup() {
  bridge.begin();
  runtime.begin(1);
  slot.begin(1, 1, 19);
  performance.onPitchBendEvent({1, 0});
  performance.onNoteEvent({NoteEventType::NoteOn, 1, 72, 100});
  performance.onNoteEvent({NoteEventType::NoteOff, 1, 72, 0});
  performance.panic();
}

void loop() {
  gate.update(false);
}
