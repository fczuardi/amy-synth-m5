# Juno Mono Patch Audition

## Goal

The polyphony probes showed that the Core Gray built-in speaker is best treated
as a monophonic AMY voice, with two-note chords only as an occasional edge
case. This slice searches for a good default Juno-style monophonic patch for
that hardware path.

## Design

The validated audio bridge remains unchanged:

- AMY renders with `AMY_AUDIO_IS_NONE`;
- the firmware paces `amy_update()`;
- AMY stereo PCM is mixed to mono;
- three rotating buffers feed `M5.Speaker.playRaw()`;
- display redraws stay out of the audio-running path;
- button A still mutes and unmutes the speaker.

The sound source returns to AMY synth patches instead of direct oscillator
tests. The firmware configures synth 1 with one voice and auditions six
candidate Juno patches:

- patch 32: Juno A51 Lead I;
- patch 33: Juno A52 Lead II;
- patch 34: Juno A53 Lead III;
- patch 75: Juno B24 Bright Pluck;
- patch 76: Juno B25 Organ Bell;
- patch 112: Juno B71 Piccolo.

Each patch plays the same C5-register phrase: notes 72, 76, 79, and 84. After
the fourth note turns off, the firmware configures the next patch. Serial logs
include both the patch number and its friendly name so listening notes can be
matched to firmware behavior.

The bridge output gain is reduced from 6x to 4x for this audition so louder
patches can be compared without overdriving the small built-in speaker.

## Boundaries

This slice does not build a reusable backend API yet. It also does not test
MIDI input, patch editing, external controls, or external audio hardware. The
only question is which built-in Juno patch should be the first default
monophonic voice candidate for the Core Gray speaker path.

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

- serial logs show `amy: patch_configured` with each candidate patch number and
  name;
- note logs show the same four-note phrase for each patch;
- `dropped` remains zero or rare;
- `blocked` remains stable or grows slowly;
- at least one patch should be clearly audible, expressive, and not too harsh
  through the built-in speaker.

## Hardware Observation

The first upload at 6x bridge gain was loud enough to ask for a lower listening
level. Reducing the bridge gain to 4x kept the patch comparison clear without
changing AMY velocity or patch behavior.

The winning patches in this audition were:

- patch 33: Juno A52 Lead II;
- patch 75: Juno B24 Bright Pluck.

These two patches are the best default monophonic voice candidates found so far
for the Core Gray built-in speaker path. Future backend/API work should start
with one of these as the default voice and keep the other as the first alternate
patch.
