# Juno demo upload recipe

## Goal

Make the current 16-channel BLE MIDI Juno audition firmware quick to rebuild
and upload while iterating on the preset selection with the Core Gray and
Arturia controller.

## Design

The repository-root `justfile` provides `upload-juno`. The recipe names both
the application directory and its `m5stack-core-gray` PlatformIO environment,
so it can be run from the repository root without remembering either value.
PlatformIO telemetry is disabled consistently with the sibling repositories.

The recipe intentionally remains a thin PlatformIO wrapper. This repository
currently targets only the known Core Gray environment for this demo, so no
multi-board selection or additional upload script is introduced.

## Verification

The command shape can be checked without accessing connected hardware:

```sh
just --list
just --dry-run upload-juno
```

The hardware validation command is:

```sh
just upload-juno
```
