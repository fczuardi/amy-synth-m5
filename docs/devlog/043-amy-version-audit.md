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

Adopt AMY `1.2.159` as the newest tested release that fits the Core Gray BLE
firmware's IRAM budget. The `amy-synth-m5` package advances to `0.2.7` with
this dependency.

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

The `1.2.166`, `1.2.165`, `1.2.162`, and `1.2.160` candidates were fetched and
tested locally. They installed and compiled, but both BLE applications failed
at link time with the same IRAM overflow: 124 bytes beyond the 131,072-byte
IRAM region.

AMY `1.2.159` is the first newer release in this sequence that links all three
applications:

- dual-channel BLE application: 130,971 of 131,072 IRAM bytes used, 101 free;
- BLE monophonic application: 130,971 used, 101 free;
- manual Juno browser without BLE: 80,607 used, 50,465 free.

The package native tests and all three application builds pass with the
candidate. The dual-channel BLE firmware was uploaded and validated on Core
Gray hardware: notes, velocity, pitch bend, CC1 modulation, channel patches,
fallback, disconnect/reconnect, panic, and idle gating remained correct. The
monophonic and manual-browser builds passed locally; no separate new hardware
session is claimed for those applications.

## Compatibility Findings

The public surface used by `amy-synth-m5` remains compatible:

- `AMY_SAMPLE_RATE`, `AMY_BLOCK_SIZE`, `AMY_NCHANS`, `amy_start`, `amy_update`,
  `amy_default_config`, `amy_default_event`, and `amy_add_event` remain usable
  by the PCM bridge and synth slot;
- `COEF_MOD` remains an alias for the original modulation input,
  `COEF_MOD0`;
- AMY adds `COEF_MOD1` and changes `mod_source` from one oscillator index to
  up to two indices. Existing one-source patches can keep using the first
  source, while a future mapping API must choose explicitly between `mod0` and
  `mod1` rather than assuming there is only one;
- `COEF_EXT0` and `COEF_EXT1` already existed in `1.2.108`. They are external
  coefficient inputs supplied by the host hook, not automatic MIDI controls,
  so this release does not require a new MIDI or modulation abstraction;
- several lower-level AMY declarations changed, including internal MIDI
  decoder helpers and `ks_note_on`/PCM signatures. Our package does not call
  those helpers directly, which is why the candidate compiles without an
  adapter rewrite.

The additional modulation source is therefore an additive AMY capability, not
a reason to expose AMY's internal coefficient arrays in our package. Our
current mapping remains intentionally narrow: it emits the tested wire command
for a patch-specific target and does not promise a universal vibrato contract.

The release also adds a runtime `max_buses` configuration. This may be a useful
future RAM experiment, but it was not changed in this audit because it is
separate from the API compatibility question and the current patches use the
existing default successfully.

The `1.2.159` dependency is now adopted in the package and applications. The
remaining 101-byte IRAM margin is a hard constraint for future feature work;
new AMY capabilities should be measured before inclusion.

Reference: <https://github.com/shorepine/amy/releases>.
