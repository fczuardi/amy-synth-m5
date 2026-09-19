# Active-channel Pitch Bend policy

## Goal

Make the 16-channel facade's Pitch Bend behavior explicit and prevent an idle
event on one channel from affecting a later note on another channel.

## Design

AMY exposes Pitch Bend as global runtime state, while
`AmyM5MonophonicSynth` presents one globally monophonic instrument selected by
16 MIDI channels. The facade therefore forwards Pitch Bend only when a note is
active and the event channel matches that note's channel. Events received
while idle or on another channel are ignored.

This restores the policy previously validated by the multichannel facade. The
16-channel refactor had accidentally retained the old one-patch mode's idle
behavior when it removed the one-patch/two-patch mode flag.

The decision is expressed by the pure `amyM5ShouldApplyPitchBend()` helper so
the host-side suite can cover the facade policy without depending on the AMY
runtime or M5 hardware.

## Verification

Native tests cover all three policy branches:

- idle events are rejected;
- an event on the active note's channel is accepted;
- an event on another channel is rejected.

The package tests and the complete Core Gray Juno firmware build are the
command-line validation targets:

```sh
pio test -d packages/amy-synth-m5 -e native
pio run -d apps/ble-midi-amy-juno -e m5stack-core-gray
```

The focused hardware regression check is to move Pitch Bend on one channel
while idle, select another channel, and confirm that its first note starts at
center pitch. Pitch Bend on the sounding note's channel must remain audible.
