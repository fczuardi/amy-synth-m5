# AMY Mono Voice Module

## Goal

The manual Juno browser proved that AMY can provide useful monophonic sounds on
the Core Gray speaker. This slice separates the AMY voice-control concern from
the hardware audition harness, without changing the validated audio bridge.

## Design

`AmyMonoVoice` is an internal module under `src/`. It owns only the musical AMY
event state for one synth:

- synth id;
- patch number;
- AMY voice count;
- active MIDI note;
- note-on/note-off state.

The main sketch still owns the hardware-facing concerns:

- button scanning;
- display setup;
- speaker setup;
- AMY startup configuration;
- PCM pacing;
- mono mixing;
- output gain;
- gate ramping;
- serial status logs.

This keeps the module small enough to understand while establishing a boundary
we can later reuse from MIDI-driven probes.

## Boundaries

This is not yet a shared library or stable package. It is a local extraction
inside `amy-synth-probe` so the next experiments can reveal whether the mono
voice API is actually the right shape.

The browser behavior stays the same:

- button A plays MIDI note 72 while held;
- button B selects the previous Juno patch;
- button C selects the next Juno patch;
- patch numbers wrap through 0 to 127.

## Verification

Command-line build:

```bash
pio run
```

Hardware target:

```bash
pio run --target upload
pio device monitor
```

Expected observation:

- the manual patch browser behaves the same as the previous slice;
- serial logs still show patch configuration and note events;
- audio quality should not change, because the audio bridge was not rewritten.
