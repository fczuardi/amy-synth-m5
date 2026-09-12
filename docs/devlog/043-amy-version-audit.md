# AMY Version Audit

## Goal

Check whether the AMY dependency used by `amy-synth-m5` is behind a newer
released version before adding more facade behavior.

## Finding

The package and all three AMY applications are pinned to AMY `1.2.108`. The
upstream GitHub release list still identifies `1.2.108` as the latest release,
so there is no newer released version to adopt in this slice.

The upstream `main` branch is not treated as an upgrade target. Consuming it
would combine unreviewed API and resource changes with a firmware path whose
Core Gray audio, BLE MIDI, pitch bend, patch fallback, modulation, and idle
behavior are already validated.

## Decision

Keep AMY pinned to `1.2.108`. Do not change the published package manifest or
the applications' dependency pins. A future upgrade spike should begin only
when a newer AMY release exists, and should compare compilation, package
consumption, IRAM, DRAM, Flash, audio behavior, and the existing hardware
checks before updating the dependency.

## Verification

The following checks passed with AMY `1.2.108`:

- native AMY package tests: 9 cases;
- package packing: `amy-synth-m5-0.2.6.tar.gz`;
- dual-channel firmware build: 129,811 bytes of IRAM used;
- BLE monophonic firmware build: 60,968 bytes of IRAM used;
- manual Juno browser build without BLE: 79,447 bytes of IRAM used.

The isolated consumer initially reused a stale cached package. After clearing
its generated state, it correctly installed local `amy-synth-m5@0.2.6`, then
failed because the transitive `fcz2/monophonic-instrument@0.1.4` dependency is
not yet available in the PlatformIO Registry. This is a distribution bootstrap
limitation, not an AMY compatibility failure. The AMY package must not regain a
machine-local dependency to hide it; publishing the matching monophonic release
is the next required distribution step.

Reference: <https://github.com/shorepine/amy/releases>.
