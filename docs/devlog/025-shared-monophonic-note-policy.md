# Shared Monophonic Note Policy

## Goal

Avoid duplicating monophonic held-note policy inside the AMY package.

The BLE MIDI AMY app needed an `InstrumentEventSink`-compatible piece before it
could graduate into an umbrella showcase. A local AMY sink would have worked,
but it would have copied policy already solved better in the
`monophonic-instrument` repository: last-note priority, held-note fallback, and
panic clearing.

## Design

The common abstraction is not `VoiceOutput` and not frequency. AMY should keep
receiving MIDI note, velocity, patch, and AMY-native pitch-bend controls.

The common abstraction is lower:

```text
NoteEvent -> monophonic held-note priority -> note action
```

The `monophonic-instrument` repository now exports `MonophonicNotePriority`.
It returns note actions with only action type, MIDI note, and velocity. Its
tone-specific `MonophonicInstrument` continues to layer frequency, waveform,
and tone pitch-bend behavior on top.

This package now adds `AmyMonophonicInstrumentSink`, which:

- implements `InstrumentEventSink`;
- uses `MonophonicNotePriority` for note on/off behavior;
- maps note actions to `AmySynthSlot`;
- maps pitch bend to `AmyRuntime`;
- wakes `AmyAudioActivityGate` when a note action or pitch bend may affect
  output.

This keeps the umbrella showcase path clean: a future showcase should be able
to compose BLE MIDI input directly into `AmyMonophonicInstrumentSink`, without
carrying app-local monophonic policy.

The name is intentionally specific. This class is not the general AMY
performance abstraction and does not represent future polyphonic or
multitimbral routing. It is the current monophonic `InstrumentEventSink`
implementation for one AMY synth slot.

## Dependency Note

`monophonic-instrument` is a package host/monorepo. Its repository root is not
a PlatformIO package; the concrete package lives at
`packages/monophonic-instrument`.

An earlier attempt to consume the Git repository root made PlatformIO treat
unrelated package directories as one library. The corrected direction is to
consume the concrete package directory or a packed archive:

```ini
lib_deps =
  monophonic-instrument=file:///home/fcz/dev/m5stick/monophonic-instrument/packages/monophonic-instrument
```

`amy-synth-m5` follows the same monorepo convention: this repository root is
not itself a package, and future exported packages should live under
`packages/<name>/`.

## Verification

```sh
pio pkg pack packages/amy-synth-m5 --output /tmp
pio run -s  # in ci/consumers/amy-synth-m5
pio run -s  # in apps/manual-juno-browser
pio run -s  # in apps/ble-midi-amy
```

Results:

- package archive generated as `/tmp/amy-synth-m5-0.1.1.tar.gz`;
- CI consumer installed `amy-synth-m5@0.1.1`;
- CI consumer installed `monophonic-instrument@0.1.2` from the sibling package
  directory;
- both local apps built successfully.

## Limits

This slice has not been hardware-validated yet. The note policy now has the
better held-note fallback behavior, so the BLE MIDI AMY app may behave slightly
better than the previous simple replacement sink when two keys are held and the
latest key is released.
