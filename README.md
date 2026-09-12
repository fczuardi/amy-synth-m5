An AMY + M5Stack package/probe repository related to
https://github.com/fczuardi/embedded-music-experiments. It is currently focused
on AMY synthesis against a M5Stack Core Gray v1.0.

The development is narrated as readable chapters in `docs/devlog/`, like the
other related projects in this young ecosystem of experimentation.

This repository is a package host/monorepo. The published package
`amy-synth-m5` is validated by the apps and devlog that remain here as
empirical evidence.

The repository root is intentionally not a PlatformIO package. Like the sibling
`monophonic-instrument` repo, this is a package host/monorepo: package
manifests live under `packages/<name>/`, and consumers should depend on those
concrete package directories or packed archives instead of the Git repository
root.

## Layout

- `apps/manual-juno-browser/`: the currently validated hardware app.
- `apps/ble-midi-amy/`: BLE MIDI input routed to AMY through the Core Gray
  speaker path.
- `packages/amy-synth-m5/`: published package for AMY synth control and the
  M5Stack Core Gray speaker bridge.
- `ci/consumers/amy-synth-m5/`: package consumer build used by CI.
- `docs/devlog/`: narrative experiment chapters.

The package boundary has graduated in place. Further work should improve the
published package through measured slices; the repository root remains a
package host rather than a PlatformIO package.

## Current State

AMY builds, uploads, boots, and renders usable synth audio on the M5Stack Core
Gray. The working path routes AMY-rendered signed 16-bit stereo PCM blocks
through `M5.Speaker.playRaw()` while AMY remains in `AMY_AUDIO_IS_NONE` mode.
This keeps the Core Gray speaker path owned by M5Unified.

The current probe is a manual AMY/Juno patch browser:

- button A plays MIDI note 72 while held;
- button B selects the previous patch;
- button C selects the next patch;
- all 128 built-in Juno patches can be auditioned on the same note.

While a note is held, buttons B and C temporarily become pitch-bend controls
for the held note, bending down/up and returning to center on release.

`packages/amy-synth-m5` is a published package. It contains AMY control helpers
plus the validated Core Gray output path: `AmyM5SpeakerBridge`
plus `AmyAudioActivityGate` own AMY PCM rendering, Core Gray speaker queuing,
and idle output shutdown.

`AmyM5MonophonicSynth` provides the simple one-slot, monophonic facade and
implements `InstrumentEventSink` directly. Lower-level consumers can still use
`AmyRuntime` for AMY-wide controls such as pitch bend, `AmySynthSlot` for one
AMY slot, and `AmyMonophonicInstrumentSink` for the shared note policy.
`MonophonicNotePriority` comes from the sibling `monophonic-instrument`
package. App code still owns BLE MIDI setup, UI, diagnostics, and hardware
composition.

The Core Gray build leaves approximately 1% of IRAM free. This is the current
practical resource limit and should be measured before adding AMY features.

One deferred experiment is per-channel modulation state. The current facade
intentionally treats the physical CC1 strip as one global performance control;
it may later remember a separate CC1 value for each configured MIDI channel and
restore it when that channel becomes active. That would remain facade policy:
AMY's underlying CC1 control is still global.

Design notes and next candidate slices live in the devlog chapters under
`docs/devlog/`.

```bash
cd apps/manual-juno-browser
pio run
```
