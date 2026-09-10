# 023 - Runtime Package Readiness Rehearsal

## Goal

Check whether `amy-synth-m5` is mechanically package-shaped enough for the
next external-consumer experiment. This does not require moving code to a new
repository: if the AMY work keeps maturing, this probe repository can itself be
renamed and hardened into the package host.

## Findings

`pio pkg pack packages/amy-synth-m5 --output .tmp` succeeds and produces:

```text
.tmp/amy-synth-m5-0.1.0.tar.gz
```

The package is mechanically packable, but its boundary is not one indivisible
abstraction. It currently contains two families:

- AMY control: `AmyRuntime`, `AmySynthSlot`;
- Core Gray output: `AmyM5SpeakerBridge`, `AmyAudioActivityGate`.

Those families have different portability. AMY control depends on Arduino plus
the AMY event API. Core Gray output depends on AMY PCM rendering, M5Unified,
and the empirically validated speaker behavior of the original M5Stack Core
Gray.

## Changes

The `library.json` manifest now records:

- repository metadata;
- package keywords;
- AMY Git dependency pinned to `1.2.108`;
- M5Unified dependency pinned to `0.2.17`;
- an explicit export list for README, headers, sources, and manifest.

The runtime README now documents the two-family package shape so future
consumers do not accidentally treat AMY control and Core Gray output as one
universal backend API.

A checked-in consumer was added under `ci/consumers/amy-synth-m5`,
following the same pattern used by `monophonic-instrument`. It depends on the
package via `file://../../../packages/amy-synth-m5` and compiles a tiny firmware
that includes all public headers and exercises the basic runtime, slot, bridge,
and gate APIs.

A minimal GitHub Actions workflow now packs the package and builds that
consumer. The BLE MIDI AMY app is intentionally not in CI yet because it still
uses a local absolute dependency on the umbrella repository for firmware
contracts.

## Design Direction

The next useful proof is not another local refactor. It is an external consumer:
the umbrella project should consume this package in a third showcase and reveal
whether the boundary survives outside the app that created it.

If that works, the likely graduation path is to keep this repository and make it
less probe-shaped:

- rename the repository if the package identity becomes clear;
- add CI for package packing and firmware builds;
- add tests where the AMY-control boundary can be exercised without hardware;
- introduce a `packages/` layout only when it removes ambiguity rather than
  merely moving files around.

The likely future split, if needed, is:

- a portable AMY-control package around runtime/slot commands;
- a hardware output package around AMY PCM into M5Unified/Core Gray.

That split should wait for the external showcase to put pressure on the API.

## Verification Target

```bash
mkdir -p .tmp
pio pkg pack packages/amy-synth-m5 --output .tmp
# Wrote a tarball to ".tmp/amy-synth-m5-0.1.0.tar.gz"

cd apps/manual-juno-browser
pio run
# [SUCCESS]

cd ../ble-midi-amy
pio run
# [SUCCESS]

cd ../../ci/consumers/amy-synth-m5
pio run
# [SUCCESS]
```

The generated tarball exports only the intended package files:

```text
README.md
include/AmyAudioActivityGate.h
include/AmyM5SpeakerBridge.h
include/AmyRuntime.h
include/AmySynthSlot.h
library.json
src/AmyAudioActivityGate.cpp
src/AmyM5SpeakerBridge.cpp
src/AmyRuntime.cpp
src/AmySynthSlot.cpp
```
