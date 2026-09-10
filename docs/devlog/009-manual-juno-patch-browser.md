# Manual Juno Patch Browser

## Goal

The automatic mono patch audition identified two strong candidates, but the
fixed four-note phrase made it harder to compare the same pitch across every
patch. This slice turns the probe into a manual patch browser so each candidate
can be heard on the exact same note.

## Design

The validated audio bridge remains unchanged:

- AMY renders with `AMY_AUDIO_IS_NONE`;
- the firmware paces `amy_update()`;
- AMY stereo PCM is mixed to mono;
- three rotating buffers feed `M5.Speaker.playRaw()`;
- display redraws stay out of the audio-running path;
- bridge output gain remains 4x.

The firmware exposes the full built-in Juno bank, patch numbers 0 through 127.
Runtime logs use patch numbers only; friendly names remain available in AMY's
generated `patches.h` comments and in the previous audition devlog shortlist.

The control surface changes:

- button A plays MIDI note 72 while held and sends note-off when released;
- button B selects the previous patch, wrapping from 0 to 127;
- button C selects the next patch, wrapping from 127 to 0.

Patch changes stop any currently active note before configuring the new patch.
Serial logs include each `patch_configured`, `note_on`, and `note_off` event
with the current patch number.

## Boundaries

This is still a probe, not a reusable backend API. It compares the built-in
Juno bank on one fixed note and does not add MIDI input or persistent settings.

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

- holding A plays note 72 for the active patch;
- releasing A stops the note;
- B and C step through all 128 Juno patches and log the selected patch number;
- the same pitch can be compared across the whole built-in Juno bank.

## Hardware Observation

Browsing the full bank on MIDI note 72 revealed many patches that work well on
the Core Gray built-in speaker:

- 19;
- 24;
- 27;
- 32;
- 33;
- 36;
- 42;
- 47;
- 53;
- 64;
- 66;
- 76.

Patches 43 and 44, which are drum/percussion-style Juno patches, also sounded
usable. This is a broader result than the first six-patch audition: the Core
Gray speaker path is not just viable for a single default lead, but can support
a small curated bank of monophonic melodic and percussive AMY/Juno voices.
