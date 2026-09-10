# Remove Output Gate

## Goal

The display redraw issue was the confirmed source of the regular audio clicks.
The output gate ramp was no longer carrying a proven requirement, so this slice
removes it before the PCM bridge accumulates policy code that may not belong
there.

## Design

The audio bridge still:

- starts AMY with `AMY_AUDIO_IS_NONE`;
- paces `amy_update()` at AMY block cadence;
- mixes AMY stereo PCM to mono;
- applies the current fixed output gain;
- feeds rotating buffers into `M5.Speaker.playRaw()`.

The removed code was limited to:

- gate constants;
- gate gain state;
- `setGateTarget()`;
- `nextGateGain()`;
- the unused `resetOutputState()`;
- the `gate=` field in status logs.

AMY note-on and note-off events are now sent directly by `AmySynthVoice`, with
no extra output envelope in the speaker bridge.

The extracted `AmySynthVoice` files also gained short comments describing the
boundary: the module owns AMY musical events only, while the experimental
`main.cpp` remains the disposable hardware harness.

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

- the manual patch browser controls remain unchanged;
- status logs no longer include `gate=`;
- if the removed ramp was not materially helping, note starts and stops should
  remain acceptable on the Core Gray speaker.
