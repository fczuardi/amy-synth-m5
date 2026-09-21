# Manual AMY drum browser

## Goal

Validate AMY's legacy General MIDI drum kit on the original M5Stack Core Gray
before using it in the planned Calculator Face step sequencer.

This first slice asks only whether patch `258` can be loaded through the
existing lower-level package components, which mapped notes are useful through
the small built-in speaker, and whether isolated hits render cleanly.

## Boundary under test

The app composes `AmyM5SpeakerBridge`, `AmyAudioActivityGate`, and
`AmySynthSlot` directly. It intentionally does not use
`AmyMonophonicInstrumentSink` or `AmyM5MonophonicSynth`: their last-note
priority, pitch-bend, channel-patch, and fallback policies describe a melodic
monophonic instrument rather than overlapping drum one-shots.

The initial slot configuration is:

```text
synth       1
patch       258 (legacy GM drum kit)
voices      1
velocity    1.0
audio tail  1500 ms
```

The one-voice value follows the kit's own built-in mapping structure and is a
hardware hypothesis to validate, not a generalized drum-kit contract.

## Interaction

Button A triggers the selected note. Buttons B and C browse the General MIDI
percussion range 35 through 81 while the audio path is idle. The screen shows
the numeric note and its conventional GM name.

Some notes may be silent or aliases because the non-Gamma9001 build contains a
reduced PCM kit. The browser preserves the full GM range precisely so those
limits can be recorded empirically.

The hit path only wakes the audio gate and queues the AMY note event. Serial
diagnostics and display work are deferred until the 1.5-second tail becomes
idle, avoiding the synchronous-I/O click previously identified in the Juno
pitch-bend probe.

## Verification

```sh
just build-drums
just upload-drums
pio device monitor --baud 115200
```

On hardware:

1. confirm that patch 258 loads without an AMY error;
2. trigger note 36 and confirm an audible isolated kick;
3. browse and trigger notes 35 through 81;
4. record silent notes, aliases, useful sounds, relative loudness, and any
   clicks or distortion;
5. confirm deferred logs report no dropped or blocked buffers during isolated
   hits.

The next slice may add controlled simultaneous layers only after this
individual-hit baseline is understood.

## Initial hardware result

Patch `258` loaded and produced audible one-shots through the Core Gray speaker.
The reduced kit is useful, but it does not behave like a high-fidelity General
MIDI drum set. Several GM notes alias very similar samples, especially in the
kick and tom ranges, while a number of electronic percussion sounds are clear,
distinctive, and musically promising.

The first listening pass classified the notes as follows:

```text
poor or nearly identical
35 36 37 41 43 45 47 48 50 65 66

good / usable
38 39 40 42 44 46 49 51 52 53 55 56 57 58 59 67 68 76 77

possibly useful; needs another musical-context test
54 60 61 62 63 64 69 70 71 72 78 79 80 81
```

Notes 35, 36, and 37 were all poor and very similar, although 37 was slightly
better. The tom-like notes were likewise difficult to distinguish. By contrast,
the snare/clap area at 38-40 was usable, and several electronic sounds were
iconic and convincing. Note 77 was particularly good but sounded almost
identical to note 68.

The follow-up check confirmed note 47 as poor and note 55 as good/usable.

This is enough evidence to continue: the Calculator sequencer need not imitate
a conventional kick/snare/tom kit. Its four tracks can instead be selected from
the strongest electronic sounds after a short overlap and pattern test.

## Controlled overlap follow-up

The second probe mode tests simultaneous one-shots without adding a clock or
pattern. A short A click triggers the current test; holding A while idle toggles
between the original single-note browser and `layers`. In layer mode, B/C
decrease or increase the number of simultaneous notes from this ordered set:

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

Each count includes all preceding entries. Triggering a selected count performs
the entire cumulative comparison: a target of three plays `1`, then `1+2`, then
`1+2+3`, with 700 ms between stages. This makes the mix immediately before and
after each addition audible in one reproducible run. The set favors sounds
classified as good in the individual pass and deliberately includes both 67
and 68 so the known near-alias behavior can be heard in a dense mix.

M5Unified's `wasClicked()` and `wasHold()` distinguish the A gestures before
either action is executed. A hold therefore changes mode without first firing a
drum. B/C and display updates remain disabled during the audio tail, and the
actual layer trigger path retains deferred diagnostics.

Hardware validation should record, for each layer count:

- whether every added sound remains perceptible;
- the first count showing clipping, clicks, or obvious loss of definition;
- deferred `dropped` and `blocked` buffer counters;
- a practical simultaneous-hit limit for the future step sequencer.

The cumulative test was then validated on the Core Gray through all eight
levels. Each successive addition could be heard through the built-in speaker,
including the final eight-sound layer. This is qualitative evidence that the
legacy kit, one `AmySynthSlot`, and the existing speaker bridge can support at
least the four coincident tracks proposed for the Calculator sequencer.

This observation does not by itself claim that every layer remained equally
distinct, that the mix was free of clipping, or that a continuously repeating
pattern has the same render behavior. Those remain separate listening and
timing questions. It does remove simultaneous one-shot capacity as an immediate
blocker for the four-track design.
