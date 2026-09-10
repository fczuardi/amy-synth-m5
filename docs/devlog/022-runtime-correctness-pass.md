# 022 - Runtime Correctness Pass

## Goal

Address the first correctness findings after the `AmyRuntime` /
`AmySynthSlot` split, without adding new abstractions.

## Changes

`AmyM5SpeakerBridge` now exposes `resumeOutput()`. The activity gate calls it
only when transitioning from idle to awake. This discards partial PCM and resets
the AMY render clock to `micros()`, so the bridge does not accumulate block
timing debt while the speaker path is deliberately asleep.

`AmyRuntime::setGlobalPitchBend()` now matches `AmySynthSlot`'s pre-`begin()`
behavior: it updates the local diagnostic value, but does not send an AMY event
until `begin()` has established the synth/channel routing metadata.

`droppedBufferCount_` now increments once per discarded stream buffer. The
bridge no longer counts both the failed `playRaw()` attempt and the discarded
buffer as separate drops.

## Design Notes

The review also clarified package shape:

- `AmyRuntime` and `AmySynthSlot` are portable AMY-control candidates.
- `AmyM5SpeakerBridge` and `AmyAudioActivityGate` are Core Gray output-path
  candidates with M5Unified and hardware-specific assumptions.
- The BLE MIDI monophonic replacement policy remains local for now; the reusable
  idea is the held-note priority policy, not an AMY-specific sink.

The ownership question for AMY startup remains open. Today the speaker bridge
starts AMY because it owns the validated PCM output path. A future external
consumer will tell us whether AMY startup belongs in `AmyRuntime` or in a more
explicit hardware runtime object.

## Verification Target

```bash
cd apps/manual-juno-browser
pio run
# [SUCCESS]

cd ../ble-midi-amy
pio run
# [SUCCESS]
```

Hardware validation target:

- first note after idle should start without variable catch-up latency;
- pitch bend behavior should remain the same after normal initialization;
- drop counters should remain meaningful if queue pressure appears.
