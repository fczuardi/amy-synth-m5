# Modulation Wheel Vibrato

## Goal

Make the Arturia modulation strip useful in the AMY facade while keeping MIDI
continuous controls independent from AMY and audio hardware.

## Design

The umbrella contract now carries `ControlChangeEvent`. `BleMidiInput` emits
it after parsing, preserving the raw zero-based channel and 7-bit values.

`AmyM5MonophonicSynth` forwards CC1 (modulation wheel) to `AmyRuntime`.
The runtime treats the value as vibrato depth, oscillating the global AMY
pitch around the current manual pitch-bend value. The first mapping is
deliberately conservative: approximately 5 Hz and up to a quarter semitone.
The modulation is applied only while a note is active and returns to the
manual bend when the note stops. Other CC numbers remain ignored by the AMY
facade, while diagnostics can still report them.

This is a facade/backend policy, not a new universal instrument abstraction.
Frequency-oriented instruments receive the generic event but need not use it.

## Verification

Native firmware-contracts and `ble-midi-input` tests cover the new event shape
and receiver forwarding seam. Firmware compilation and physical confirmation
of audible vibrato remain required before this slice is considered hardware
validated.
