# Fixed 16-channel AMY configuration

## Goal

Replace the experimental one-channel and two-channel facade modes with one
explicit configuration for all MIDI channels. This is an intentional
backwards-incompatible alpha change.

## Design

`AmyM5MonophonicSynthConfiguration` contains one AMY synth id, one voice count,
and exactly 16 `uint16_t` patch entries. The facade copies the configuration in
`begin()`, so the caller's storage may be temporary. MIDI event channels remain
zero-based: `patches[0]` is physical MIDI channel 1 and `patches[15]` is
physical MIDI channel 16. Patch zero is valid; malformed channels are rejected
by the pure channel-resolution helper and by the facade.

The facade still owns one AMY synth slot and one global monophonic note policy.
Sixteen selectable patches are not sixteen voices or a multitimbral engine.
Note identity and fallback remain global across channels, and a fallback loads
the returning note's channel patch before the note action is applied.

CC1 remains one global physical performance-control value. After a note
transition, the facade waits for the audio bridge to render a block before
replaying that value, preserving the validated AMY patch-load ordering. Juno
modulation registration now performs one compound mapping attempt for each of
the 16 channels. Each mapping contains the three relative tonal oscillator
commands because AMY accepts only one mapping per channel/controller pair.

## Verification

Native tests cover first, last, and middle channel resolution, patch zero, the
malformed-channel guard, and the fixed 16-channel count. The existing
monophonic policy tests cover note identity, repeated notes, wrong-channel
release, fallback, panic, and disconnect; the facade's AMY ordering remains an
integration concern because it depends on the AMY C runtime and speaker bridge.
Firmware builds cover the complete facade and all local applications. The
representative hardware check is to use Shift+keys 1 through 16 on the Arturia
controller, confirm each channel selects its configured patch, and exercise
cross-channel fallback, CC1, pitch bend, panic, and disconnect behavior. The
Juno registration loop deliberately makes one compound mapping attempt per
channel; each attempt contains the three tonal-oscillator commands rather than
three competing mappings.

The Core Gray Juno build after all 16 mappings are registered used:

- IRAM: 130,971 / 131,072 bytes, 101 bytes free (99.92%);
- DRAM: 62,912 / 124,580 bytes, 61,668 bytes free (50.50%);
- application flash: 1,210,955 / 3,145,728 bytes (38.5%);
- generated firmware image: 1,231,543 bytes.

The IRAM result matches the previous 1.2.159 baseline and links successfully,
but leaves no practical headroom for arbitrary additions. The extra mapping
state fits in DRAM. Any future expansion must measure this same firmware path.

## Preset audition

The initial table assigned Juno patch 58, `A83 Drum Booms`, to physical MIDI
channel 8. Hardware audition with the Arturia found that notes played through
this percussion-oriented patch sounded too similar to one another for the
intended general-purpose selection. Patch 62, `A87 FX Sweep`, was tried next
but was also rejected during hardware audition. Channel 8 now uses patch 54,
`A77 Sustainer`, as a more conventionally pitched candidate. The other 15
channel assignments remain fixed so this change can be evaluated in isolation.

The Arturia/Core Gray audition accepted patch 54 as a useful default and the
complete table as a good initial 16-preset demo set:

| Physical MIDI channel | Juno patch |
| ---: | ---: |
| 1 | 0 |
| 2 | 9 |
| 3 | 18 |
| 4 | 24 |
| 5 | 32 |
| 6 | 40 |
| 7 | 49 |
| 8 | 54 |
| 9 | 64 |
| 10 | 73 |
| 11 | 80 |
| 12 | 89 |
| 13 | 96 |
| 14 | 105 |
| 15 | 114 |
| 16 | 120 |

This closes selection of the initial demo bank. Future preset substitutions
can be evaluated as independent audition slices rather than holding back the
16-channel configuration API.

## Outside this slice

The umbrella Showcase 3 remains on its published package version until this
new API is validated and published. Updating that consumer is a separate
propagation slice. Per-channel modulation state, multitimbrality, polyphony,
and the unproven AMY `mod1` route remain outside this change.
