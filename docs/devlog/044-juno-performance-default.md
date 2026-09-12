# Juno performance modulation default

## Goal

Reduce the configuration required to use the validated AMY Juno performance
modulation without making applications know the oscillator layout of each
patch.

## Design

`AmyM5MonophonicSynth::configureJunoPerformanceModulation()` is a convenience
operation for the facade. It registers CC1 as a second AMY modulation input on
the relative tonal oscillators 2, 3, and 4 for every configured MIDI channel.
The AMY Juno patch already connects its relative oscillator 1 LFO to those
sources, so the control changes only the temporary performance coefficient in
`mod1` and leaves the patch's stored `mod0` behavior intact.

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
build are the command-line checks for this slice. Hardware validation should
confirm that CC1 still modulates patches 19 and 24 independently before a new
package release is published.
