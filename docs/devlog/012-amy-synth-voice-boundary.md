# AMY Synth Voice Boundary

## Goal

The first extracted wrapper was named `AmyMonoVoice`, but note replacement is a
monophonic instrument policy rather than an AMY event-wrapper concern. This
slice renames the wrapper to `AmySynthVoice` and keeps it closer to AMY's
performance-event model.

## Design

`AmySynthVoice` still owns one AMY synth slot:

- synth id;
- configured AMY voice count;
- active patch number;
- latest MIDI note started through the wrapper;
- whether that latest note is still considered active.

The wrapper no longer stops an existing note inside `noteOn()`. It exposes:

- `noteOn(midiNote, velocity)`;
- `noteOff(midiNote)`;
- `stopActiveNote()`.

That keeps note-on/note-off as direct AMY events and leaves monophonic
replacement policy in the caller. The current browser still behaves
monophonically because its button flow only starts one fixed audition note and
stops it before changing patches.

## Boundaries

This is still an internal probe module, not a package API. The rename is meant
to keep the local boundary honest while we learn whether the future shared
contract should look like a synth-performance backend rather than a frequency
tone output.

## Design Direction

AMY should not be forced into the existing tone-output shape from the sibling
experiments. The buzzer and simple speaker paths are frequency outputs: an
instrument policy chooses a frequency and waveform, then the backend renders
that tone. AMY is a synth backend: it can represent higher-level musical
intent such as patch/program, MIDI note, velocity, pitch bend, and eventually
multi-channel or richer synth controls.

The local `AmySynthVoice` wrapper is justified as a thin ownership boundary
around the AMY API. It should wrap our repeated usage pattern, not hide AMY:

- own one AMY synth slot;
- track configured patch/program and latest active note;
- translate simple performance methods into `amy_event` messages;
- avoid speaker, PCM buffering, display, button, or hardware policy;
- avoid monophonic replacement policy inside `noteOn()`.

The PCM bridge is a separate lower-level concern. It owns AMY block pacing,
stereo-to-mono conversion, fixed output gain, rotating buffers, and
`M5.Speaker.playRaw()`. It may become package-worthy later, but it should first
survive more than one real AMY usage mode.

Existing contracts in sibling repos remain flexible. `VoiceOutput` and
`MonophonicInstrument` are empirical checkpoints, not legacy APIs. AMY may
motivate a new performance-oriented contract rather than fitting under the
current frequency-tone interface.

Candidate follow-up slices:

- add pitch-bend support to `AmySynthVoice` and validate it audibly on a known
  good Juno patch;
- build a MIDI-driven AMY showcase:
  `BLE MIDI receiver -> instrument/performance policy -> AmySynthVoice -> AMY
  PCM bridge -> M5.Speaker`;
- decide whether note-priority policy should remain in `MonophonicInstrument`,
  move into an AMY-specific instrument adapter, or be replaced by a more
  general performance-event sink;
- extract the PCM bridge locally into a named internal module after one more
  usage mode proves the current bridge shape;
- promote any AMY package only after at least two consumers reveal a stable
  contract;
- keep Core Gray hardware limits explicit: monophonic AMY/Juno patches are
  proven useful; richer chords were not strong enough on the built-in speaker
  to drive the immediate roadmap.

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
- button A still plays note 72 while held;
- buttons B and C still browse the 128 AMY/Juno patches;
- serial logs still report patch and note events.
