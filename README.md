A sandbox repo related to
https://github.com/fczuardi/embedded-music-experiments meant to be the home of
initial AMY library explorations against a M5Stack Core Gray v1.0.

The development is narrated as readable chapters in `docs/devlog/`, like the
other related projects in this young ecosystem of experimentation.

Later this can graduate to a more organized package, or be renamed, if the
probing and experimentation shows potential.

## Current State

The first slice validated that AMY builds, uploads, boots, and reports
`amy_started=true` on the M5Stack Core Gray.

The current slice routes AMY-rendered signed 16-bit stereo PCM blocks through
`M5.Speaker.playRaw()` while AMY remains in `AMY_AUDIO_IS_NONE` mode. This keeps
the Core Gray speaker path owned by M5Unified and avoids guessing direct I2S
pins before local sound is proven.

```bash
pio run
```
