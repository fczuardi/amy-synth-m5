# Manual Drum Browser

Hardware probe for AMY's legacy General MIDI drum kit (`patch 258`) through the
M5Stack Core Gray built-in speaker.

Controls:

- a short button A click triggers the current single note or layer set;
- holding button A while idle switches between `single` and `layers` modes;
- buttons B/C select the previous/next GM note in `single` mode;
- buttons B/C decrease/increase the simultaneous count in `layers` mode.

The browser covers GM notes 35 through 81. A displayed GM name does not promise
that the reduced legacy kit contains a distinct sample for that note; audible,
silent, duplicated, and poorly reproduced notes are all relevant hardware
observations.

The trigger path deliberately performs no serial logging or display redraw.
Diagnostics are deferred until the 1.5-second audio tail closes so synchronous
I/O cannot be mistaken for an AMY or speaker artifact.

Build and upload from the repository root:

```sh
just build-drums
just upload-drums
pio device monitor --baud 115200
```

Layer mode compares cumulative combinations from an ordered test set of eight
sounds that were useful in the individual hardware pass:

```text
38 Acoustic Snare
42 Closed Hi-Hat
55 Splash Cymbal
56 Cowbell
59 Ride Cymbal 2
67 High Agogo
68 Low Agogo
76 Hi Wood Block
```

For a selected count of three, one A click plays `1`, then `1+2`, then `1+2+3`,
with 700 ms between stages. This makes the sound before and after each addition
audible in the same run. Increase the target count from one through eight and
listen for loss of definition, clipping, clicks, or missing hits. Buttons are
ignored while the comparison and its audio tail are active. Continuous-pattern
limits remain a later slice after this controlled simultaneous test.
