# amy-synth-m5

AMY synth helpers and M5Stack speaker bridge code shared by the local apps.

This package is still maturing, but it is published for consumption outside
the original probe apps. The umbrella Showcase 3 exercises its simple
monophonic facade, while the lower-level AMY/M5 boundaries remain available for
further experiments.

## Simple Monophonic Use

`AmyM5MonophonicSynth` is the convenience facade for the common case: one AMY
slot, the shared monophonic note policy, and the Core Gray speaker output. The
application initializes M5Unified first, then uses the facade as its MIDI
event sink:

```cpp
AmyM5MonophonicSynth synth;

void setup() {
  M5.begin(M5.config());
  synth.begin(1, 1, 19);
  bleMidiInput.setInstrumentEventSink(&synth);
}

void loop() {
  bleMidiInput.update();
  synth.update();
}
```

The lower-level runtime, slot, gate, bridge, and sink headers remain available
for applications that need different composition defaults.

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

## Package Candidate Shape

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
`fcz2/monophonic-instrument@0.1.2`, so external consumers do not need a sibling
checkout or a local `file://` dependency for the shared monophonic note policy.

## Tests

```sh
pio test -e native
```

The native tests currently cover pure pitch-bend clamping and conversion. The
AMY/M5 speaker path remains covered by firmware builds and hardware validation.
