A sandbox repo related to
https://github.com/fczuardi/embedded-music-experiments meant to be the home of
initial AMY library explorations against a M5Stack Core Gray v1.0.

The development is narrated as readable chapters in `docs/devlog/`, like the
other related projects in this young ecosystem of experimentation.

Later this can graduate to a more organized package, or be renamed, if the
probing and experimentation shows potential.

## Current Slice

The first slice is an AMY dependency and build probe for the M5Stack Core Gray.
It initializes M5Unified and starts AMY in no-audio mode so dependency,
toolchain, link, flash, and RAM friction are visible before any sound-output
adapter is designed.

```bash
pio run
```
