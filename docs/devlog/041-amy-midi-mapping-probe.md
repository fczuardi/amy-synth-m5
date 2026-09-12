# AMY MIDI Mapping Probe

## Goal

Test AMY's own MIDI mapping mechanism on Juno patch 19 before introducing any
modulation policy in the package facade.

## Setup

The dual-channel app now uses the local `ble-midi-input` package so parsed
Control Change events reach the app. It registers one AMY mapping at startup:

```text
AMY MIDI channel 1, CC1 -> v2f,,,,,%vZ
```

The mapping targets oscillator 2, the first sounding oscillator in the Juno
patch structure, and changes only its `freq` `mod` coefficient. Empty
coefficient fields preserve the other frequency controls. The value range is
0.0 to 0.1 octaves. Channel 2 and other controllers are deliberately not
mapped.

The app forwards the typed event back into AMY's mapping engine as a complete
MIDI CC message. This is intentionally app-local probe code, not package
behavior or a new universal synth interface.

## Verification

The dual-channel Core Gray firmware builds successfully with the new local
receiver and contract packages. Hardware validation should check whether
holding a note on channel 1 and moving the modulation strip produces vibrato,
whether the depth follows the strip, and whether channel 2 remains unchanged.
No audible result is claimed yet.
