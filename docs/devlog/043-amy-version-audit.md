# AMY Version Audit

## Goal

Check whether the AMY dependency used by `amy-synth-m5` is behind a newer
released version before adding more facade behavior.

## Finding

The package and all three AMY applications are pinned to AMY `1.2.108`. The
upstream release page currently shows AMY `1.2.166` as the latest release, so
there is a real newer version to evaluate. An earlier web lookup returned stale
release data and incorrectly concluded that `1.2.108` was current; this chapter
corrects that conclusion.

The upstream `main` branch is not treated as an upgrade target. Consuming it
would combine unreviewed API and resource changes with a firmware path whose
Core Gray audio, BLE MIDI, pitch bend, patch fallback, modulation, and idle
behavior are already validated.

## Decision

Keep the published package pinned to `1.2.108` until `1.2.166` completes an
isolated upgrade check. The candidate must be compared for compilation,
package consumption, IRAM, DRAM, Flash, audio behavior, and the existing
hardware checks before any dependency update.

## Verification

The following checks passed with the existing AMY `1.2.108` dependency:

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

The `1.2.166` upgrade test remains pending because it requires fetching and
building the newer upstream dependency in the local PlatformIO environment.

Reference: <https://github.com/shorepine/amy/releases>.
