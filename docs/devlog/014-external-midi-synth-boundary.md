# External MIDI Synth Boundary

## Goal

The AMY probe has started revealing a performance-oriented boundary based on
patches, MIDI notes, velocity, and pitch bend rather than the frequency-only
shape used by the existing tone outputs. This note compares that emerging
boundary with a materially different implementation: the M5Stack Unit Synth,
which delegates synthesis and audio output to a SAM2695 hardware chip.

This is design prior art, not a new dependency or a commitment to acquire or
support the Unit Synth.

## SAM2695 as a contrasting implementation

The SAM2695 contains its own General MIDI wavetable, synthesis firmware, voice
allocation, effects, and stereo DAC. The Unit Synth accepts MIDI over a UART
and adds an amplifier plus a small speaker. The host microcontroller does not
render PCM blocks.

The official Arduino driver exposes operations for:

- instrument selection by bank, channel, and program;
- note on and note off with channel, pitch, and velocity;
- all notes off;
- pitch bend and pitch-bend range;
- master and per-channel volume, expression, and pan;
- reverb, chorus, tuning, vibrato, filter, envelope, modulation, and reset.

AMY reaches audio through a very different path:

```text
performance calls -> amy_event -> AMY renderer -> PCM bridge -> M5.Speaker
```

The Unit Synth path is:

```text
performance calls -> MIDI bytes over UART -> SAM2695 -> audio output
```

Despite the implementation difference, both benefit from retaining musical
intent such as note, velocity, program, channel, bend, and cleanup instead of
reducing every request to a frequency.

## Design findings

The comparison supports the current decision not to force AMY behind the
frequency-oriented `VoiceOutput` contract.

It also clarifies several vocabulary boundaries:

- a patch is engine-specific state;
- MIDI Program Change is a transport-independent musical request that an
  engine may map to one of its patches or presets;
- pitch-bend position and pitch-bend range are separate concerns;
- channel can carry concrete musical behavior for multitimbral engines;
- all-notes-off or panic is an instrument cleanup operation, not a frequency
  command;
- PCM rendering and hardware audio output are separate from performance
  control, even when one library or chip hides that separation.

The official Unit Synth API is useful evidence but is not a candidate universal
interface. It combines standard MIDI messages, device-specific SysEx controls,
and UART transport in one driver. A shared ecosystem contract should remain
smaller and should grow only when a real producer and consumer require a new
event such as Program Change or Control Change.

## Consequences for this probe

`AmySynthVoice` remains an internal AMY wrapper. It should continue translating
the small performance surface currently exercised by hardware without trying
to reproduce the complete Unit Synth API.

The comparison does not yet justify:

- adding Program Change to the shared firmware contracts;
- adding generic patch, effect, or synth-capability interfaces;
- importing MIDI transport into this probe;
- packaging the AMY wrapper or PCM bridge;
- treating the SAM2695 as a planned backend.

A later MIDI-driven AMY composition can provide the second real consumer needed
to decide whether a broader performance-oriented interface should be shared.

## Sources

- [M5Stack Unit Synth](https://docs.m5stack.com/en/unit/Unit-Synth)
- [SAM2695 datasheet](https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/docs/products/unit/Unit-Synth/SAM2695.pdf)
- [M5Unit-Synth Arduino API](https://github.com/m5stack/M5Unit-Synth/blob/main/src/M5UnitSynth.h)
- [General MIDI instrument definitions](https://github.com/m5stack/M5Unit-Synth/blob/main/src/M5UnitSynthDef.h)
