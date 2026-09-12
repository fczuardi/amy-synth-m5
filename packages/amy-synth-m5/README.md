# amy-synth-m5

AMY synth helpers and M5Stack speaker bridge code shared by the local apps.

This package is still alpha, but is published for consumption outside the
original probe apps. The facade configures one AMY synth slot with a fixed
patch selection for all 16 MIDI channels while remaining globally monophonic.

Version 0.3.0 intentionally removes the earlier one-channel and two-channel
`begin()` APIs. There is no backwards-compatibility layer in this alpha.

## Simple Monophonic Use

`AmyM5MonophonicSynth` is the convenience facade: one AMY slot, one global
monophonic note policy, and the Core Gray speaker output. The application
initializes M5Unified first, then supplies exactly 16 patch entries:

```cpp
AmyM5MonophonicSynth synth;
AmyM5MonophonicSynthConfiguration configuration{
    .synthId = 1,
    .voiceCount = 1,
    .patches = {
        19, 24, 7, 31,
        42, 55, 68, 73,
        80, 88, 96, 104,
        112, 120, 126, 127,
    },
};

void setup() {
  M5.begin(M5.config());
  synth.begin(configuration);
  synth.configureJunoPerformanceModulation();
  bleMidiInput.setInstrumentEventSink(&synth);
}

void loop() {
  bleMidiInput.update();
  synth.update();
}
```

`patches[0]` is physical MIDI channel 1 and `patches[15]` is physical MIDI
channel 16. All 16 channels are always supported. Selecting a different
channel selects a timbre in the same slot; it does not create 16 voices or a
multitimbral instrument. The monophonic policy remains global, so held notes
across channels participate in the same last-note priority and fallback.

The configuration is copied by `begin()`, so its lifetime does not constrain
the synth. Patch identifiers are `uint16_t`, and patch zero is valid.

The lower-level runtime, slot, gate, bridge, and sink headers remain available
for applications that need different composition defaults.

`configureJunoPerformanceModulation()` is an optional convenience default. It
uses AMY's fixed Juno oscillator layout for CC1: the patch LFO on relative
oscillator 1 modulates the frequency of tonal oscillators 2, 3, and 4 through
the hardware-validated `mod0` route. It registers one compound mapping per
channel, because AMY accepts only one mapping for a channel/controller pair.
`configureMidiControlMapping()` remains available for patches or targets
outside that layout.

## Current Boundaries

`AmyM5SpeakerBridge` owns the low-level audio runtime:

- starts AMY with `AMY_AUDIO_IS_NONE`;
- paces `amy_update()` at AMY block cadence;
- mixes AMY stereo `int16_t` PCM to mono;
- applies the empirically validated Core Gray output gain;
- queues fixed-size buffers into `M5.Speaker.playRaw()`;
- resets block pacing when output resumes after an intentional idle period;
- exposes diagnostic counters for rendered, queued, dropped, and blocked
  buffers.

`AmyAudioActivityGate` owns the Core Gray output lifecycle:

- wakes the bridge when the app expects audible sound;
- keeps a short release tail after notes end;
- stops the M5Unified speaker channel at rest to avoid idle hiss.

`AmyRuntime` owns AMY-wide musical controls:

- stores the synth/channel id used as routing metadata for runtime events;
- sends AMY's global pitch-bend event;
- ignores outgoing commands until `begin()` establishes that routing metadata;
- keeps the current global bend value for diagnostics.

`AmyPitchBend` owns the pure MIDI-to-AMY pitch-bend mapping used by
`AmyRuntime` and covered by native tests.

`AmySynthSlot` owns a thin AMY musical-control slot:

- configures one AMY synth slot;
- selects built-in patches;
- sends note on/off events.

`AmyMonophonicInstrumentSink` applies shared monophonic note policy to AMY:

- implements `InstrumentEventSink` from `firmware-contracts`;
- uses `MonophonicNotePriority` from `monophonic-instrument`;
- maps monophonic note actions to AMY note on/off events;
- maps MIDI velocity to AMY note velocity;
- maps `PitchBendEvent` to AMY's global pitch bend through `AmyRuntime`;
- wakes the audio gate whenever a musical event may produce output;
- exposes active-note and pitch-bend state for diagnostics.

## Non-Goals

This library does not own BLE MIDI, application UI, patch browsing policy,
polyphonic allocation policy, or generic cross-board audio output.

## Package Shape

This package currently contains two families with different portability:

| Family | Components | Assumptions |
| --- | --- | --- |
| AMY control | `AmyRuntime`, `AmySynthSlot`, `AmyMonophonicInstrumentSink` | Arduino + firmware contracts + monophonic note policy + AMY event API |
| Core Gray output | `AmyM5SpeakerBridge`, `AmyAudioActivityGate` | AMY PCM, M5Unified, Core Gray speaker behavior |

The audio bridge captures the validated Core Gray PCM path. `AmyRuntime` and
`AmySynthSlot` remain lower-level APIs for experiments that need more control;
the external Showcase 3 currently consumes the higher-level monophonic facade.

The manifest declares both AMY and M5Unified because the current package ships
both families together. If the AMY-control family graduates separately later,
it should not carry the M5Unified dependency.

`AmyMonophonicInstrumentSink` also requires `monophonic-instrument`. The
manifest declares the public PlatformIO Registry package
`fcz2/monophonic-instrument@0.1.5`, so external consumers do not need a sibling
checkout or a local `file://` dependency for the shared monophonic note policy.

## Tests

```sh
pio test -e native
```

The native tests cover pure pitch-bend clamping/conversion and the fixed
channel-to-patch configuration helper. The AMY/M5 speaker path remains covered
by firmware builds and hardware validation.
