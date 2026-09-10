# Juno Polyphony Limit Probe

## Goal

The previous slice proved that a built-in AMY Juno patch can play clearly
through the validated Core Gray speaker bridge. This slice asks the next small
hardware-limit question: how does the same bridge behave as Juno polyphony
increases from one to four simultaneous notes?

## Design

The audio bridge remains unchanged:

- AMY renders with `AMY_AUDIO_IS_NONE`;
- the firmware paces `amy_update()` at the AMY block cadence;
- AMY stereo PCM is mixed to mono;
- three rotating buffers feed `M5.Speaker.playRaw()`;
- the display is not redrawn during audio status updates.

The sound source remains AMY synth 1 with Juno patch 1 and four allocated
voices. The local sequencer now cycles through chord sizes:

- 1 voice: note 48;
- 2 voices: notes 48 and 52;
- 3 voices: notes 48, 52, and 55;
- 4 voices: notes 48, 52, 55, and 60.

The output gain is reduced from 12x to 6x for this slice so stacked voices are
less likely to overload the Core Gray internal speaker path. The serial status
line reports active chord size plus render, queue, drop, and blocked counters.

## Boundaries

This is not yet a musical arrangement, MIDI receiver, patch browser, or dynamic
voice allocator. It is only a local AMY stress probe for the old Core Gray
speaker bridge.

## Verification

Command-line build:

```bash
pio run
```

Hardware target:

```bash
pio run --target upload
pio device monitor
```

Expected observation:

- serial logs show `amy: chord_on voices=1`, then 2, 3, and 4;
- `dropped` remains zero or rare;
- `blocked` remains stable or grows slowly;
- the Core Gray speaker remains recognizable as chord size increases.

Failure signals include audible breakup on larger chords, rapidly increasing
`dropped`, rapidly increasing `blocked`, or obvious speaker distortion even
after reducing output gain.

## Hardware Observation

The serial counters stayed healthy during the run. Rendering stayed near the
expected AMY cadence, `dropped` stayed at zero, and `blocked` only moved slowly.
That means the old ESP32 and the M5Unified bridge were not obviously failing at
four Juno voices.

The audible limit arrived earlier than the counters did. One and two voices
were usable, but the built-in Core Gray speaker path started to deteriorate
after the second Juno voice, and three- and four-note chords sounded noticeably
worse. For this hardware, the practical built-in-speaker limit for this Juno
patch is therefore closer to one or two voices, even though the renderer can
keep up with four voices.
