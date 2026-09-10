# Triangle Chord Limit Probe

## Goal

The basic waveform probe found that all C5 tones were clear on the Core Gray
built-in speaker, while C4 sine was weak, C4 triangle was better, and C4 square
was excessive. This slice asks whether triangle can support simple AMY chords
better than the previous sine chord probe.

## Design

The validated audio bridge remains unchanged:

- AMY renders with `AMY_AUDIO_IS_NONE`;
- the firmware paces `amy_update()`;
- AMY stereo PCM is mixed to mono;
- three rotating buffers feed `M5.Speaker.playRaw()`;
- display redraws stay out of the audio-running path.

The sound source uses direct AMY oscillators configured as `TRIANGLE`. The chord
cycle uses the same major-chord shape as earlier probes, moved into the clear
C5 range:

- 1 voice: note 72;
- 2 voices: notes 72 and 76;
- 3 voices: notes 72, 76, and 79;
- 4 voices: notes 72, 76, 79, and 84.

Each voice owns one oscillator. MIDI note numbers are converted locally to
frequency so the probe does not depend on AMY synth patch allocation.

## Boundaries

This slice does not test Juno patches, envelopes, drums, MIDI input, or musical
voice management. It is only a practical triangle-polyphony limit test for the
Core Gray speaker bridge.

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

- serial logs show `amy: triangle_on` and `amy: triangle_off` for each
  oscillator;
- `dropped` remains zero or rare;
- `blocked` remains stable or grows slowly;
- the single triangle voice should remain clear;
- if two or more triangle voices still sound musical, triangle is a better
  built-in-speaker chord baseline than sine;
- if three or four voices become robotic or smeared, the practical built-in
  speaker chord limit is still low even with a friendlier waveform and register.

## Hardware Observation

The Core Gray built-in speaker made the 1-voice and 2-voice triangle steps
passable. The 3-voice and 4-voice steps did not reveal much additional chord
detail; they mostly sounded stronger in volume with a robotic, slightly
detuned-like character.

This suggests triangle is a better baseline than sine for the built-in speaker,
but the practical polyphony limit is still about two voices. Three or four AMY
voices may be rendered correctly by the firmware, but they are not musically
clear through this speaker path.
