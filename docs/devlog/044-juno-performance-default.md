# Juno performance modulation default

## Goal

Reduce the configuration required to use the validated AMY Juno performance
modulation without making applications know the oscillator layout of each
patch.

## Design

`AmyM5MonophonicSynth::configureJunoPerformanceModulation()` is a convenience
operation for the facade. It registers the package's known patch profiles for
the configured MIDI channels, using CC1 and AMY frequency modulation with the
validated coefficient range.

The first profiles are deliberately small and empirical:

| AMY patch | Relative target oscillator |
| --- | ---: |
| 19 | 3 |
| 24 | 2 |

An unknown patch is not guessed. Its channel receives no automatic mapping and
the method reports failure if no known profile could be configured. The lower-
level `configureMidiControlMapping()` remains available for new measurements
or mappings that are not part of the package defaults.

This table is an integration detail of the AMY version used by the package,
not a claim that all Juno-derived patches share one universal oscillator
topology.

## Verification

The dual-channel AMY app was changed to use the convenience operation instead
of two hand-written mappings. Native package tests and the Core Gray firmware
build are the command-line checks for this slice. Hardware validation should
confirm that CC1 still modulates patches 19 and 24 independently before a new
package release is published.
