# 057 — Hide M5 trigger-output lifecycle plumbing

## Goal

Let a trigger composition depend on one musical output object instead of
constructing the AMY speaker bridge, activity gate, synth slot, and lane sink.

## Facade

`AmyM5TriggerOutput` implements `TriggerEventSink` and owns those four backend
objects. `begin()` starts the bridge and a default General MIDI drum slot,
applies speaker volume, and accepts an optional configuration for applications
that need another synth slot, patch, voice count, or volume.

The facade also forwards lane configuration and exposes `update()` for the
audio render/tail lifecycle. It deliberately does not call `M5.begin()`:
display, buttons, microphone, and other board-wide choices still belong to the
application.

## Validation

The electronic-drummer firmware compiles with only `AmyM5TriggerOutput` visible
at its audio boundary. Hardware playback remains the acceptance target because
the facade changes concrete initialization and ownership paths.
