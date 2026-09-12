# AMY MIDI Mapping Probe

## Goal

Test AMY's own MIDI mapping mechanism on Juno patch 19 before introducing any
modulation policy in the package facade.

## Setup

The dual-channel app now uses the local `ble-midi-input` package so parsed
Control Change events reach the app. It registers one AMY mapping at startup:

```text
AMY MIDI channel 1, CC1 -> i%iv3f,,,,,%vZ
AMY MIDI channel 2, CC1 -> i%iv2f,,,,,%vZ
```

The mappings target the audible oscillator in each patch relative to synth 1
and change only its `freq` `mod` coefficient. Patch 19 uses oscillator 3,
while patch 24 uses oscillator 2.
The `i%iv3` prefix is important: without it, `v3` addresses a global AMY
oscillator rather than oscillator 3 within the allocated synth voice. Empty
coefficient fields preserve the other frequency controls. The value range is
0.0 to 0.1 octaves. Other controllers are deliberately not mapped. The first
attempt used oscillator 2 for both patches, but patch 19 gives that oscillator
no effective amplitude.

The facade translates the typed control-change event into a complete MIDI CC
message for AMY's native mapping engine. The application still supplies the
patch-specific oscillator choices, but it no longer constructs AMY wire
messages or depends on AMY's mapping functions directly.

## Verification

The dual-channel Core Gray firmware builds successfully with the new local
receiver and contract packages. The Core Gray hardware then produced audible
levels of pitch variation on a held channel 1 note as the Arturia modulation
strip moved. The effect was consistent with the AMY patch's LFO being applied
with a depth controlled by CC1. This validates the AMY-native mapping path for
this patch and setup.

The mapping data remains application configuration because oscillator roles are
patch-specific. The package facade owns the translation and registration
mechanism, without imposing a universal modulation policy on every patch.
