A sandbox repo related to
https://github.com/fczuardi/embedded-music-experiments meant to be the home of
initial AMY library explorations against a M5Stack Core Gray v1.0.

The development is narrated as readable chapters in `docs/devlog/`, like the
other related projects in this young ecosystem of experimentation.

Later this can graduate to more organized packages, or be renamed, if the
probing and experimentation shows potential.

## Layout

- `apps/manual-juno-browser/`: the currently validated hardware app.
- `apps/ble-midi-amy/`: BLE MIDI input routed to AMY through the Core Gray
  speaker path.
- `lib/amy-probe-runtime/`: internal helpers shared by local probe apps.
- `docs/devlog/`: narrative experiment chapters.

The repository root is intentionally left free for future `packages/` if an
AMY boundary matures into reusable exported modules.

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

`lib/amy-probe-runtime` is now the internal staging area for AMY boundaries
that have more than one local consumer. The strongest package candidate is the
audio bridge: `AmyM5SpeakerBridge` plus `AmyAudioActivityGate` own AMY PCM
rendering, Core Gray speaker queuing, and idle output shutdown.

`AmySynthVoice` is also extracted, but remains more provisional. It owns one
AMY synth slot and translates patch, note, and pitch-bend calls into AMY
events, while app code still owns performance policy such as monophonic note
replacement and BLE MIDI routing.

Design notes and next candidate slices live in the devlog chapters under
`docs/devlog/`.

```bash
cd apps/manual-juno-browser
pio run
```
