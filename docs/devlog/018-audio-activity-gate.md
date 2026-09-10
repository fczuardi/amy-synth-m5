# Audio Activity Gate

## Goal

The idle speaker fix worked, but both apps carried the same wake/tail/idle-stop
logic. This slice moves that policy into the internal `amy-probe-runtime`
library without promoting it to an exported package.

## Design

`AmyAudioActivityGate` wraps an `AmyM5SpeakerBridge` and owns the output
lifecycle policy:

- `wake()` marks recent musical activity and starts a default `700 ms` release
  tail;
- `update(soundActive)` renders while a note is active or the release tail is
  still open;
- `forceIdle()` stops the speaker channel immediately.

The apps still decide what counts as musical activity. Note On, Note Off, Pitch
Bend, and panic wake the gate. The manual browser also calls `forceIdle()` while
muted.

## Boundaries

This helper is deliberately internal to the probe. It is not an AMY musical
voice and not a generic instrument contract. It captures a hardware-output
policy discovered on the Core Gray speaker path.

## Verification

Commands:

```bash
cd apps/ble-midi-amy
pio run

cd ../manual-juno-browser
pio run
```

Both builds passed.
