# 031. Monophonic facade

## Goal

Reduce the setup burden for the common AMY use case without collapsing the
lower-level runtime, slot, audio bridge, or activity gate boundaries.

## Design

`AmyM5MonophonicSynth` is a convenience facade for one monophonic AMY
instrument on an M5 speaker. It owns the existing runtime, synth slot, speaker
bridge, activity gate, and `AmyMonophonicInstrumentSink`, and implements
`InstrumentEventSink` so a MIDI receiver can target it directly. The facade
also hides AMY's global pitch-bend detail from this simple one-instrument use
case.

The existing component headers remain public for applications that need a
different composition. The application still initializes `M5Unified` before
calling the facade's `begin()` method.

## Verification

```sh
pio test -e native
pio pkg pack packages/amy-synth-m5 --output /tmp
```

The umbrella Showcase 3 was compiled against the updated package from a local
dependency path before publication. The final showcase manifest remains on
the published package version until `amy-synth-m5` 0.1.2 is released.

No new hardware validation was performed in this slice. The previous
hardware-validated MIDI, velocity, pitch-bend, reconnect, panic, and idle-gate
behavior remains the reference behavior for the facade migration.
