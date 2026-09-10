# amy-probe-runtime

Internal AMY runtime helpers shared by the local probe apps.

This library is package-shaped on purpose, but it is not an exported package
yet. It exists to keep the two current AMY apps from duplicating the same Core
Gray audio path while the reusable boundary is still being tested.

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

## Non-Goals

This library does not own BLE MIDI, application UI, patch browsing policy,
monophonic note replacement, or generic cross-board audio output.

The audio bridge is the strongest extraction candidate so far. `AmyRuntime`
and `AmySynthSlot` are intentionally kept more provisional until another AMY
showcase proves whether this runtime/slot split is the right reusable package
shape.
