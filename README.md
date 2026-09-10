A sandbox repo related to
https://github.com/fczuardi/embedded-music-experiments meant to be the home of
initial AMY library explorations against a M5Stack Core Gray v1.0.

The development is narrated as readable chapters in `docs/devlog/`, like the
other related projects in this young ecosystem of experimentation.

Later this can graduate to a more organized package, or be renamed, if the
probing and experimentation shows potential.

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

`AmySynthVoice` is the first extracted boundary. It is intentionally thin: it
owns one AMY synth slot and translates patch/note calls into AMY events, while
`main.cpp` remains the disposable hardware harness.

Design notes and next candidate slices live in the devlog chapters under
`docs/devlog/`.

```bash
pio run
```
