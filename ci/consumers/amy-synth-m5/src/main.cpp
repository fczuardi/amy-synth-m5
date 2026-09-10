#include <Arduino.h>

#include "AmyAudioActivityGate.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyMonophonicInstrumentSink.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"

AmyM5SpeakerBridge bridge;
AmyAudioActivityGate gate(bridge);
AmyRuntime runtime;
AmySynthSlot slot;
AmyMonophonicInstrumentSink instrumentSink(runtime, slot, gate);

void setup() {
  bridge.begin();
  runtime.begin(1);
  slot.begin(1, 1, 19);
  instrumentSink.onPitchBendEvent({1, 0});
  instrumentSink.onNoteEvent({NoteEventType::NoteOn, 1, 72, 100});
  instrumentSink.onNoteEvent({NoteEventType::NoteOff, 1, 72, 0});
  instrumentSink.panic();
}

void loop() {
  gate.update(false);
}
