#include <Arduino.h>

#include "AmyM5MonophonicSynth.h"

AmyM5MonophonicSynth amySynth;

const AmyM5MonophonicSynthConfiguration configuration{
    .synthId = 1,
    .voiceCount = 1,
    .patches = {
        19, 24, 7, 31,
        42, 55, 68, 73,
        80, 88, 96, 104,
        112, 120, 126, 127,
    },
};

void setup() {
  amySynth.begin(configuration);
  amySynth.onPitchBendEvent({0, 0});
  amySynth.onNoteEvent({NoteEventType::NoteOn, 0, 72, 100});
  amySynth.onNoteEvent({NoteEventType::NoteOff, 0, 72, 0});
  amySynth.panic();
}

void loop() {
  amySynth.update();
}
