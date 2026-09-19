# Publish amy-synth-m5 0.3.1

## Goal

Publish the hardware-validated correction for residual Pitch Bend state as a
small patch release before returning to the umbrella Showcase 3.

## Release contents

Version `0.3.1` keeps the 16-channel, globally monophonic API introduced in
`0.3.0`. It changes only Pitch Bend handling around a physical controller's
center position:

- non-center Pitch Bend remains rejected while the instrument is idle;
- values in the existing `-128..128` center dead zone are normalized to exact
  zero;
- an idle center event may therefore clear AMY's global bend before the next
  note.

The adjacent Juno-app change suppressing high-rate Pitch Bend serial output is
outside the published package. It was nevertheless included in the firmware
hardware validation because synchronous logging had produced an audible click.
A broader best-effort diagnostic design remains queued as a separate
architecture slice.

## Verification

Before tagging, run:

```sh
pio test -d packages/amy-synth-m5 -e native
pio pkg pack packages/amy-synth-m5 --output /tmp
pio run -d ci/consumers/amy-synth-m5
pio run -d apps/ble-midi-amy-juno -e m5stack-core-gray-ci
```

Hardware validation on the Core Gray with the Arturia controller confirmed
that releasing a bent note and then centering the strip no longer bends the
next note. With per-event Pitch Bend logging removed from the Juno app,
centering the strip while a note remains held also returns without a click.

Publication is triggered by pushing the annotated tag
`amy-synth-m5-v0.3.1`. The workflow validates that the tag and manifest
versions match before publishing the package to the PlatformIO Registry.
