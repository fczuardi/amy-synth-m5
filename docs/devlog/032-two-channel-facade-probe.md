# Channel Patch Selection Probe

## Goal

Test whether the existing Core Gray AMY facade can use two MIDI channels to
select patches while remaining one global monophonic instrument.

## Design

`AmyM5MonophonicSynth` now has one-patch and two-patch initialization forms. In
the two-patch form, MIDI event channels 0 and 1, corresponding to physical MIDI
channels 1 and 2, select two patches on one AMY slot. One
`AmyMonophonicInstrumentSink` owns the single global held-note policy.

The patch changes on the next Note On, so a currently sounding note is not
reconfigured merely because a controller changes its MIDI channel.

## Probe App

`apps/ble-midi-amy-dual-channel` uses patches 19 and 24 for physical MIDI
channels 1 and 2. It logs selected and configured patches and exposes Button A
as a panic control. The existing `apps/ble-midi-amy` remains unchanged and
continues to exercise the one-patch facade used by Showcase 3.

## Verification

Both firmware applications built successfully with:

```sh
pio run -e m5stack-core-gray
```

The one-patch and two-patch builds both remain at 129811 bytes of IRAM. The
two-patch build uses 60896 bytes of DRAM, compared with 60816 bytes for the
one-patch application before this simplification. Physical validation of this
new global-monophonic behavior is pending.

The published `amy-synth-m5@0.1.2` package and the umbrella Showcase 3 remain
unchanged until the two-channel behavior is validated on the Core Gray.
