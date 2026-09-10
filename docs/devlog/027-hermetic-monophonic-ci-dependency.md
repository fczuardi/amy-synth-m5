# Hermetic Monophonic CI Dependency

## Goal

Restore `amy-synth-m5` CI without treating the sibling
`monophonic-instrument` repository root as a PlatformIO package.

## Problem

`AmyMonophonicInstrumentSink` depends on `MonophonicNotePriority`, which lives
in the concrete PlatformIO package:

```text
monophonic-instrument/packages/monophonic-instrument
```

Local AMY builds used an absolute developer-machine path:

```ini
monophonic-instrument=file:///home/fcz/dev/m5stick/monophonic-instrument/packages/monophonic-instrument
```

That path is valid on the local workstation but does not exist inside GitHub
Actions. The remote CI failure was therefore expected: PlatformIO raised a
`FileNotFoundError` for a machine-specific path.

## Design

The temporary hermetic CI path is explicit checkout composition:

- check out `fczuardi/amy-synth-m5`;
- check out `fczuardi/monophonic-instrument` into `deps/monophonic-instrument`;
- pin that checkout to the validated commit
  `9b558750a0a43432705de6d8d31ee8a16f209310`;
- point the isolated AMY consumer at:

```ini
monophonic-instrument=file://../../../deps/monophonic-instrument/packages/monophonic-instrument
```

This preserves the monorepo boundary: neither repository root becomes a
PlatformIO package. The dependency is still the concrete package directory.

## Verification

Local verification mirrored the CI layout by cloning the sibling repository
into `deps/monophonic-instrument` and checking out the pinned commit.

```sh
pio pkg pack packages/amy-synth-m5 --output /tmp
pio run -s  # in ci/consumers/amy-synth-m5
```

Results:

- package archive generated as `/tmp/amy-synth-m5-0.1.1.tar.gz`;
- consumer installed `monophonic-instrument@0.1.2` from
  `../../../deps/monophonic-instrument/packages/monophonic-instrument`;
- consumer build passed.

## Follow-up

This was a bootstrap CI path, not the final distribution mechanism. It still
required checking out the sibling repository. The intended release path was a
custom GitHub Release asset containing the packed subdirectory tarball, not
GitHub Packages and not GitHub's automatic repository source archive.

That follow-up is handled in the release-asset slice once
`monophonic-instrument-v0.1.2` exists.
