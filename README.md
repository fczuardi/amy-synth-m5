An AMY + M5Stack package/probe repository related to
https://github.com/fczuardi/embedded-music-experiments. It is currently focused
on AMY synthesis against a M5Stack Core Gray v1.0.

The development is narrated as readable chapters in `docs/devlog/`, like the
other related projects in this young ecosystem of experimentation.

This repository is graduating in place from probe toward package host. The
current package candidate is `amy-synth-m5`; the apps and devlog remain here as
empirical validation.

The repository root is intentionally not a PlatformIO package. Like the sibling
`monophonic-instrument` repo, this is a package host/monorepo: package
manifests live under `packages/<name>/`, and consumers should depend on those
concrete package directories or packed archives instead of the Git repository
root.

## Layout

- `apps/manual-juno-browser/`: the currently validated hardware app.
- `apps/ble-midi-amy/`: BLE MIDI input routed to AMY through the Core Gray
  speaker path.
- `packages/amy-synth-m5/`: package candidate for AMY synth control and the
  M5Stack Core Gray speaker bridge.
- `ci/consumers/amy-synth-m5/`: package consumer build used by CI.
- `docs/devlog/`: narrative experiment chapters.

The repository is now allowed to graduate in place: if the package boundary
keeps holding, this repo can be renamed and hardened rather than replaced by a
sibling package repository.

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

`packages/amy-synth-m5` is the current package candidate. It contains AMY
control helpers plus the validated Core Gray output path: `AmyM5SpeakerBridge`
plus `AmyAudioActivityGate` own AMY PCM rendering, Core Gray speaker queuing,
and idle output shutdown.

`AmyRuntime` and `AmySynthSlot` are also extracted, but remain more
provisional. The runtime owns AMY-wide controls such as pitch bend. The slot
owns one AMY synth slot and translates patch and note calls into AMY events,
`AmyMonophonicInstrumentSink` adapts shared `InstrumentEventSink` events into a
single AMY synth slot by reusing `MonophonicNotePriority` from the sibling
`monophonic-instrument` package. App code still owns BLE MIDI setup, UI,
diagnostics, and hardware composition.

Design notes and next candidate slices live in the devlog chapters under
`docs/devlog/`.

```bash
cd apps/manual-juno-browser
pio run
```
