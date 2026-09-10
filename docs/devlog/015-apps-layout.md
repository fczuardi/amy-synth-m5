# Apps Layout

## Goal

The next experiment should consume existing BLE MIDI packages without
duplicating their code. Before adding that showcase, this slice reorganizes the
repo so runnable firmware lives under `apps/`, matching the sibling
`monophonic-instrument` layout and leaving room for future `packages/`.

## Design

The validated manual browser moved from the repository root to:

```text
apps/manual-juno-browser/
```

The moved app keeps the same source files and behavior:

- `src/main.cpp`;
- `src/amy_synth_voice.h`;
- `src/amy_synth_voice.cpp`;
- `platformio.ini`.

Only the PlatformIO cache path changed. From the app directory, `core_dir` now
points to the shared workspace cache at:

```ini
core_dir = ../../../.platformio-home
```

The root is no longer the PlatformIO project. It stays focused on repo-level
documentation and can later host `packages/` if `AmySynthVoice`, the PCM
bridge, or another boundary earns promotion.

## Boundaries

This slice does not add BLE MIDI and does not package any AMY code. It is a
layout preparation step so the next app can consume `ble-midi-input` as a real
package instead of copying BLE code into the probe.

## Verification

Command-line build from the moved app:

```bash
cd apps/manual-juno-browser
pio run
```

Expected result:

- the firmware builds with the same dependencies;
- the app remains the manual AMY/Juno browser validated in previous slices.
