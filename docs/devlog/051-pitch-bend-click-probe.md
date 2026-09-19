# Pitch-bend click probe

## Goal

Identify whether the click heard when the Arturia pitch strip returns to center
comes from AMY's pitch transition or from work performed by the Juno demo while
processing the controller's burst of Pitch Bend messages.

## Observation

The controller does not jump directly from its positive bend to center. Serial
diagnostics show a descending sequence of Pitch Bend values that settles at
approximately `64`. The facade's center dead zone normalizes that final value
to zero.

The Juno demo was also formatting and writing one serial line for every value
in that dense sequence. `BleMidiInput` invokes that diagnostic synchronously
before forwarding the same event to the instrument. A blocking serial write is
therefore a plausible source of an audio interruption, independent of the
pitch calculation itself.

## Experiment

Suppress per-event Pitch Bend logging in `ble-midi-amy-juno`. Note, connection,
control-change, panic, and dropped-event diagnostics remain enabled. The MIDI
event stream delivered to `AmyM5MonophonicSynth` is unchanged.

This deliberately does not add a second pitch ramp. The physical controller
already supplies a gradual return, and adding smoothing before isolating the
diagnostic overhead would combine two variables.

## Verification

Build and upload the Juno app:

```sh
just upload-juno
```

On the Core Gray, hold a note, move the pitch strip upward, then release the
strip while continuing to hold the note. Compare the return to center with the
previous firmware:

- if the click disappears, the high-rate serial diagnostic was disturbing the
  audio path;
- if it remains, investigate AMY pitch updates and smoothing as the next
  isolated experiment.

Also repeat the residual-bend regression: release the note while the strip is
raised, release the strip, then play another note. The new note must start at
center pitch.

## Hardware result

The firmware was uploaded to the Core Gray and tested with the Arturia
controller. Holding a note, bending upward, and releasing the strip while the
note remained held no longer produced the click.

This confirms that the artifact came from synchronous per-event serial logging
during the strip's dense Pitch Bend burst, rather than from AMY's direct pitch
update. No additional smoothing or change to Pitch Bend response is needed.

## Follow-up

The Juno app still logs other MIDI facts synchronously, including Control
Change events that can also arrive in dense bursts. A near-term architecture
slice should prototype fixed-size, best-effort diagnostics: keep formatting and
serial writes outside the musical event path, coalesce high-rate values, and
drop diagnostics rather than delaying MIDI delivery or audio rendering. The
design should first be proven in this app and `ble-midi-amy` before promotion
to a shared contract.
