# AMY MIDI Mapping Probe

## Goal

Test AMY's own MIDI mapping mechanism on two Juno patches and expose the
smallest useful mapping configuration through the package facade.

## Setup

The dual-channel app now uses the local `ble-midi-input` package so parsed
Control Change events reach the app. It registers one AMY mapping per patch at
startup:

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
message for AMY's native mapping engine. It forwards the event only when its
channel corresponds to the patch currently selected in the shared synth slot.
Channel support is reported separately from the patch number, so AMY patch 0
remains a valid selectable patch rather than serving as an unsupported-channel
sentinel.
The application still supplies the patch-specific oscillator choices, but it
no longer constructs AMY wire messages or depends on AMY's mapping functions
directly.

## Verification

The dual-channel Core Gray firmware builds successfully with the new local
receiver and contract packages. The Core Gray hardware produced audible levels
of pitch variation on held notes for both configured patches as the Arturia
modulation strip moved. The effect was consistent with each AMY patch's LFO
being applied with a depth controlled by CC1.

Hardware validation also confirmed that the mapping remains tied to the active
patch: changing channels selects the corresponding patch, and a control-change
event for the other channel does not modify the currently active patch.

The native test suite covers the pure AMY message formatter for all supported
targets and invalid input cases. The firmware build remains at 129,811 bytes of
IRAM used, with 1,261 bytes free.

The mapping data remains application configuration because oscillator roles are
patch-specific. The package facade owns the translation, registration, and
active-patch guard, without imposing a universal modulation policy on every
patch.
