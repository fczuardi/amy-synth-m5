# Modulation Wheel Vibrato

## Goal

Make the Arturia modulation strip useful in the AMY facade while keeping MIDI
continuous controls independent from AMY and audio hardware.

## Design

The umbrella contract now carries `ControlChangeEvent`. `BleMidiInput` emits
it after parsing, preserving the raw zero-based channel and 7-bit values.

The initial implementation briefly attempted to turn CC1 into an oscillator in
our runtime by repeatedly emitting pitch-bend events. That was rejected: it
recreated synth behavior outside AMY, changed the meaning of global pitch bend,
and did not use the LFO routing encoded by an AMY patch.

The generic event remains in the shared contract and receiver, but the AMY
facade currently ignores it. A future vibrato slice must first identify an AMY
native patch/LFO control path and then expose only the smallest backend-specific
operation needed to use it.

## Verification

Native firmware-contracts and `ble-midi-input` tests cover the event shape and
receiver forwarding seam. No vibrato hardware validation is claimed.
