# Juno performance modulation default

## Goal

Reduce the configuration required to use the validated AMY Juno performance
modulation without making applications know the oscillator layout of each
patch.

## Design

`AmyM5MonophonicSynth::configureJunoPerformanceModulation()` is a convenience
operation for the facade. It maps CC1 onto the existing AMY `mod0` route of the
relative tonal oscillators 2, 3, and 4 for every configured MIDI channel.
Those oscillators already use the relative oscillator 1 LFO through their
existing `mod0` route in AMY's Juno patches, so the facade changes the
coefficient on that established route. AMY allows one mapping per channel and
controller, so the three oscillator updates are emitted as one concatenated
mapping template rather than registered as three competing mappings.

An initial attempt used the new `mod1` input from AMY 1.2.159 and configured a
second LFO source dynamically. The firmware built successfully, but the first
hardware test produced no audible modulation. The unproven `mod1` route was
abandoned for this slice, returning the profile to the known-working `mod0`
path. Preserving patch-authored modulation through `mod1` remains a separate
AMY upgrade investigation. A second hardware test then exposed the
one-mapping-per-controller rule; this slice fixes that by composing the three
updates into one mapping.

Fallback note transitions also restore the current global control values only
after the monophonic sink has completed the new note action. This ordering is
important because a fallback can reload the selected patch before starting
the previous note again. The facade therefore defers the replay until the
audio bridge has rendered a block after the note transition. This lets AMY
finish the patch-load deltas before the three-command Juno mapping is replayed.
Normal incoming controls remain immediate.

This follows the fixed Juno oscillator topology rather than a table of
individually measured patches:

| Relative oscillator | Role |
| --- | --- |
| 1 | LFO / modulation source |
| 2 | pulse / PWM |
| 3 | saw |
| 4 | sub oscillator |

The lower-level `configureMidiControlMapping()` remains available for patches
or targets that do not follow this Juno layout. The profile is intentionally
specific to AMY's Juno patch family, not a universal policy for all AMY
patches.

## Verification

The dual-channel AMY app was changed to use the convenience operation instead
of two hand-written mappings. Native package tests and the Core Gray firmware
build pass against AMY 1.2.159. The first hardware upload did not produce
audible modulation; the missing `mod1` source route was then corrected. Core
Gray hardware validation confirmed modulation on both patches, including
cross-channel fallback after deferring control replay until a rendered block.
