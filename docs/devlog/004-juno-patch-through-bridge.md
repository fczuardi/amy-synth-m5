# Juno Patch Through Bridge

## Goal

The previous slice validated the speaker bridge with a single AMY sine
oscillator. This slice keeps that bridge intact and changes only the AMY sound
source: instead of a raw oscillator, the firmware configures AMY synth 1 with a
built-in Juno patch and sends MIDI-style note events through it.

## Design

The audio path remains unchanged:

- AMY renders with `AMY_AUDIO_IS_NONE`;
- firmware calls `amy_update()` at the AMY block cadence;
- AMY stereo PCM is mixed to mono;
- mono samples pass through the 12x guarded output gain and short output gate;
- three rotating buffers feed `M5.Speaker.playRaw()`;
- the display is not redrawn during audio status updates.

The sound source now follows AMY's own synth examples. During AMY startup, the
firmware configures synth 1 with patch 1 and four voices. The test sequencer
cycles MIDI notes 48, 52, 55, and 60 through that synth.

## Boundaries

This slice does not enable BLE MIDI, UART MIDI, drums, or a full song example.
It also does not promote the bridge into a reusable audio backend. The only
question is whether an AMY built-in patch can use the already-validated Core
Gray speaker bridge.

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

- serial logs show `amy: synth_configured synth=1 patch=1 voices=4`;
- note logs show `amy: note_on` and `amy: note_off` for the four-note cycle;
- `dropped` remains zero or rare;
- the Core Gray speaker plays a recognizable AMY/Juno-style patch without the
  display-related clicks fixed in the previous slice.

## Hardware Observation

The uploaded firmware played the AMY Juno patch clearly on the Core Gray
built-in speaker. The four-note cycle was recognizable, the patch character was
audible, and the display-related clicks from earlier probes did not return.

That validates the bridge beyond the raw sine test: AMY built-in synth patches
can render through the paced mono `M5.Speaker.playRaw()` path on this hardware.
