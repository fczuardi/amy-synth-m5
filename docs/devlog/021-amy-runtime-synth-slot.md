# 021 - AMY Runtime and Synth Slot Split

## Goal

Avoid mapping AMY concepts too early onto our own backend names. The previous
`AmySynthVoice` wrapper mixed three concerns:

- a per-AMY-synth-slot patch/note command surface;
- app-level active-note bookkeeping;
- AMY's pitch-bend command, which behaves like shared runtime state.

This slice separates those concerns while keeping the two current apps
behaviorally equivalent.

## Design

`AmyRuntime` now owns AMY-wide musical controls. For now that only means global
pitch bend, using the same MIDI bend mapping already validated in the probe.
It also stores the synth/channel id used as routing metadata, because AMY's own
MIDI pitch-bend path tags the bend event with the source channel even though
the resulting bend is stored globally.

`AmySynthSlot` now owns only one AMY synth slot:

- `begin(synthId, voiceCount, initialPatch)`;
- `setPatch(patchNumber)`;
- `noteOn(midiNote, velocity)`;
- `noteOff(midiNote)`.

It intentionally does not track whether a note is active. The manual browser
and BLE MIDI app now own their local active-note state because that state is
performance policy: button audition behavior in one app, monophonic MIDI note
replacement and panic handling in the other.

## Boundary Notes

The split gives us a cleaner candidate package shape without pretending that
AMY is a generic frequency backend. AMY still receives musical events in its
own terms: patches, MIDI notes, velocities, slots, and global bend.

The interesting open question is whether `AmyRuntime` should eventually own
audio startup as well. For this slice, `AmyM5SpeakerBridge` still starts AMY in
`AMY_AUDIO_IS_NONE` mode because that path is already validated and remains
more audio-backend-specific than musical-control-specific.

The apps now consume the local runtime through `lib_extra_dirs = ../../lib`
instead of installing it through `lib_deps`. That keeps local probe apps wired
to the working tree during refactors, avoiding stale copied headers under
`.pio/libdeps`.

## Verification Target

Command-line validation:

```bash
cd apps/manual-juno-browser
pio run

cd ../ble-midi-amy
pio run
```

Expected hardware behavior after upload:

- manual browser still auditions note 72 on button A;
- buttons B/C still cycle patches when no note is held;
- buttons B/C still bend down/up while the note is held;
- BLE MIDI AMY still responds to note on/off, velocity, pitch bend, and panic.

## Command-Line Verification

Both local consumers build after the split:

```bash
cd apps/manual-juno-browser
pio run
# [SUCCESS]

cd ../ble-midi-amy
pio run
# [SUCCESS]
```

Hardware upload/audible validation is still pending for this slice.
