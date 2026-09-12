# AMY MIDI Mapping Probe

## Goal

Test AMY's own MIDI mapping mechanism on Juno patch 19 before introducing any
modulation policy in the package facade.

## Setup

The dual-channel app now uses the local `ble-midi-input` package so parsed
Control Change events reach the app. It registers one AMY mapping at startup:

```text
AMY MIDI channel 1, CC1 -> i%iv3f,,,,,%vZ
```

The mapping targets oscillator 3 relative to synth 1, which is an audible
oscillator in Juno patch 19, and changes only its `freq` `mod` coefficient.
The `i%iv3` prefix is important: without it, `v3` addresses a global AMY
oscillator rather than oscillator 3 within the allocated synth voice. Empty
coefficient fields preserve the other frequency controls. The value range is
0.0 to 0.1 octaves. Channel 2 and other controllers are deliberately not
mapped. The first attempt targeted oscillator 2, but patch 19 gives that
oscillator no effective amplitude.

The app forwards the typed event back into AMY's mapping engine as a complete
MIDI CC message. This is intentionally app-local probe code, not package
behavior or a new universal synth interface.

## Verification

The dual-channel Core Gray firmware builds successfully with the new local
receiver and contract packages. The Core Gray hardware then produced audible
levels of pitch variation on a held channel 1 note as the Arturia modulation
strip moved. The effect was consistent with the AMY patch's LFO being applied
with a depth controlled by CC1. This validates the AMY-native mapping path for
this patch and setup.

The mapping remains app-local. No modulation policy was added to the package
facade, and channel 2 was not mapped in this probe.
