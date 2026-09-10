# amy-synth-m5

AMY synth helpers and M5Stack speaker bridge code shared by the local apps.

This package is still maturing, but it is now shaped for consumption outside
the original probe apps. It keeps the current AMY/M5 boundary explicit while a
future umbrella showcase tests whether the API is stable enough to publish.

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

The audio bridge is the strongest extraction candidate so far because it
captures the validated Core Gray PCM path. `AmyRuntime` and `AmySynthSlot` are
also promising, but they should be tested from an external showcase before
being treated as a stable public AMY-control API.

The manifest declares both AMY and M5Unified because the current package ships
both families together. If the AMY-control family graduates separately later,
it should not carry the M5Unified dependency.

`AmyMonophonicInstrumentSink` also requires `monophonic-instrument`, but this
package does not currently declare that dependency in `library.json` because
the sibling repo is a package host whose Git root is not itself a PlatformIO
package. Until a registry, archive, or subpackage distribution path is chosen,
consumers must provide `packages/monophonic-instrument` explicitly.
