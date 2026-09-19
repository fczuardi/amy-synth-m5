# Remove facade patch mutation

## Goal

Keep the 16-channel configuration as the single source of truth for patch
selection and correct the application descriptions in the repository README.

## Design

`AmyM5MonophonicSynth::setPatch()` was left over from the earlier one-patch
facade. In the fixed 16-channel facade it changed only the currently loaded
patch and `selectedPatch_`; the next Note On loaded that channel's configured
patch again. The temporary mutation therefore had misleading semantics and is
removed before publishing version 0.3.0.

The lower-level `AmySynthSlot::setPatch()` remains available and continues to
serve the manual Juno browser. Applications using the facade select patches by
constructing `AmyM5MonophonicSynthConfiguration` before `begin()`.

The root README now completes the Core Gray speaker-path description on the
`ble-midi-amy` item and leaves the `ble-midi-amy-juno` item as the 16-channel
patch-selection and modulation app.

## Verification

The package native tests, isolated package consumer, and complete Juno firmware
must all build:

```sh
pio test -d packages/amy-synth-m5 -e native
pio run -d ci/consumers/amy-synth-m5
pio run -d apps/ble-midi-amy-juno -e m5stack-core-gray
```

No new hardware behavior is introduced by this API cleanup.

## Future direction

A live-performance UI may eventually need to change the patch assigned to one
specific MIDI channel at runtime. That operation would be explicit about the
channel, conceptually `setPatchForMidiChannel(channel, patch)`, rather than
temporarily replacing the patch loaded in the shared AMY slot.

That API is deliberately deferred until an M5 screen-and-button experiment can
answer whether changing the active channel applies immediately or on the next
Note On, how it interacts with a sounding note, and whether assignments persist
across reboot. The immutable-after-`begin()` configuration remains the honest
0.3.0 boundary.
