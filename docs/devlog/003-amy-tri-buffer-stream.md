# AMY Tri-Buffer Stream Probe

## Goal

The previous slice proved that the Core Gray built-in speaker path is clean when
driven directly by `M5.Speaker.tone()`. It also showed that AMY-rendered sine
tones were recognizable but contaminated by radio-like artifacts when routed
through the first `playRaw()` bridge.

This slice tests whether a more stream-like bridge fixes that problem while
still letting M5Unified own the Core Gray speaker output.

## Design

AMY is back in `AMY_AUDIO_IS_NONE` mode. The firmware calls `amy_update()` for
rendered PCM blocks and feeds them to M5Unified using `M5.Speaker.playRaw()`.

The bridge now follows M5Unified's own generated-audio guidance more closely:

- use three rotating buffers;
- keep each buffer alive after queuing it;
- queue at most one pending buffer into the M5Unified virtual channel;
- preserve the Core Gray default mono DAC speaker configuration;
- convert AMY's stereo PCM block to mono before calling `playRaw()`.

Each stream buffer holds three AMY blocks, or 768 mono samples. That keeps the
buffer size aligned with AMY's render cadence while staying close to the small
streaming buffers used by M5Unified examples.

The first tri-buffer hardware check was still very noisy. Only one of the
cycled tones sounded clearly tonal, likely the 440 Hz tone, so the probe was
narrowed again. It now plays only a 440 Hz AMY sine tone, lowers AMY velocity,
and attenuates the mono PCM before handing it to `playRaw()`. This separates
gain or clipping problems from stream transport problems and avoids judging the
tiny built-in speaker at lower frequencies while the artifact is unresolved.

The fixed-frequency attenuated probe was quieter but still noisy. The serial
log revealed the real bridge defect: the main loop was calling `amy_update()`
far faster than audio time. At 44.1 kHz and 256-frame AMY blocks, the expected
render cadence is about 172 blocks per second, but the firmware was rendering
over 1000 blocks per second and dropping thousands of full stream buffers while
the M5Unified channel stayed full. The bridge now paces AMY rendering with a
microsecond scheduler derived from `AMY_BLOCK_SIZE / AMY_SAMPLE_RATE`.

The first paced monitor run confirmed that runaway rendering stopped:
`dropped_buffers` and `pacing_blocked` remained at zero and the speaker queue
held steady. It also showed the scheduler was drifting slow, rendering about
162-163 blocks per second instead of the expected 172 blocks per second. The
probe now advances the scheduler by the ideal block interval and allows a small
catch-up burst after display or serial work, capped at three AMY blocks per loop
so it cannot return to the original uncontrolled render rate.

The drift-corrected run reached the expected render cadence, about 172 blocks
per second, but exposed a smaller queue-boundary bug. The bridge could render
the AMY block that completed a 768-sample stream buffer even when the M5Unified
channel already had both current and next buffers occupied. That full buffer
then failed to queue and was dropped. The queue guard now treats a
buffer-completing AMY block as requiring immediate M5Unified queue space.

The same run still sounded very quiet and noisy. The monitor showed
`peak_sample` around 59, which is far below a useful 16-bit PCM signal level and
mostly exercises the Core Gray speaker noise floor. The next probe keeps the
corrected timing and queue guard but restores AMY velocity to 1.0 and removes
the extra PCM attenuation. The expected signal should still be far from
clipping, but much easier to judge by ear.

Restoring AMY velocity removed the deliberate attenuation, but hardware still
sounded quiet and noisy. The monitor showed the unattenuated AMY sine peak near
1193, still only a small fraction of 16-bit full scale. The next probe applies
a fixed 12x PCM gain with saturation before handing samples to M5Unified. It
also logs both `input_peak` and the amplified `peak_sample` so clipping is
visible in the monitor.

The 12x gain probe made the AMY tone clearly audible, but regular clicks
remained. The monitor showed stable transport: no dropped buffers and stable
input/output peaks. The next likely source is hard amplitude gating from
immediate sine on/off events. The firmware now starts the AMY sine oscillator
once and applies a short PCM gain ramp in the bridge for note on/off. This keeps
AMY rendering continuous while our output gate fades in and out over 1024
samples.

The clicks still appeared to happen during the held tone, and by observation
they lined up with display changes rather than note boundaries. The firmware
had been redrawing the whole screen once per second in the status block. That
periodic `fillScreen()` and text redraw is now removed from the audio-running
path. The display still updates during setup and when Button A toggles mute, but
regular status reporting is serial-only.

The hardware check confirmed the diagnosis: removing periodic display redraws
removed the audible held-tone clicks. With paced AMY rendering, tri-buffered
mono `playRaw()` output, 12x guarded PCM gain, and no display redraw in the
audio-running path, the AMY sine probe is now clearly audible without the
regular interruptions heard earlier.

After validation, the firmware was simplified to keep only the working bridge
concerns: AMY setup, paced rendering, three output buffers, stereo-to-mono mix,
12x guarded output gain, a short output gate ramp, Button A mute, and serial
health counters. Earlier peak diagnostics and verbose display status were
removed.

Button A toggles mute and unmute so the device can stay flashed during
listening tests.

## Boundaries

This slice does not attempt to:

- restore AMY synth patches;
- receive BLE MIDI;
- support polyphony;
- expose reusable audio backend APIs;
- take direct ownership of ESP32 I2S or DAC output.

If this bridge is clean, a later slice can move from sine back to AMY patches.
If it is still noisy, the next likely investigation is direct I2S/DAC ownership
instead of `M5.Speaker.playRaw()`.

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

- serial logs show `amy_started=true`;
- `rendered` increases near the AMY audio cadence, about 170 blocks per second;
- `queued_buffers` and `queued_samples` increase steadily;
- `dropped_buffers` and `pacing_blocked` remain zero or rare;
- Button A toggles `muted=true` and `muted=false`;
- the speaker plays a clean intervalled 440 Hz sine tone without the radio-like
  artifacts heard in the first AMY bridge;
- `peak_sample` stays well below 16-bit full scale, confirming the bridge is not
  obviously clipping before M5Unified receives the buffer.
