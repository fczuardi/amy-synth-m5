# Basic Waveform Speaker Probe

## Goal

The Juno polyphony probe showed that the bridge and ESP32 render path can keep
up with four voices, but the Core Gray built-in speaker path starts sounding
poor after about two Juno voices. This slice started by asking whether that
limit is caused mostly by the rich Juno patch or by simple summed polyphony in
general.

After the sine chord tests, the goal narrowed further: identify a very basic
single-oscillator waveform and pitch range that the Core Gray speaker can
reproduce clearly before testing chords again.

## Design

The validated bridge stays unchanged:

- AMY renders with `AMY_AUDIO_IS_NONE`;
- the firmware paces `amy_update()`;
- AMY stereo PCM is mixed to mono;
- three rotating buffers feed `M5.Speaker.playRaw()`;
- display redraws stay out of the audio-running path.

The sound source changes from AMY synth/Juno patch voices to plain sine
oscillators. The first attempt used the same low chord shape as the Juno
polyphony probe:

- 1 voice: note 48;
- 2 voices: notes 48 and 52;
- 3 voices: notes 48, 52, and 55;
- 4 voices: notes 48, 52, 55, and 60.

On hardware, even the single low sine voice did not read clearly as a tone.
Before changing waveform, the probe moves the same chord shape up one octave:

- 1 voice: note 60;
- 2 voices: notes 60 and 64;
- 3 voices: notes 60, 64, and 67;
- 4 voices: notes 60, 64, 67, and 72.

This keeps the waveform simple and isolates whether the built-in speaker path
is especially weak for low sine fundamentals. If the higher sine is still not
clear, the next useful comparison is a triangle or square wave, because those
add harmonics that small speakers reproduce more easily.

On hardware, the higher single sine became more audible, but still quiet. The
stacked sine sounds were robotic and did not read as a chord. That suggests
plain summed sine polyphony is not a useful musical baseline for this built-in
speaker path.

The current probe therefore returns to one oscillator and cycles six single
tones:

- sine, triangle, and square at MIDI note 60;
- sine, triangle, and square at MIDI note 72.

`PULSE` uses an explicit 50 percent duty cycle, so it acts as the square-wave
test. MIDI note numbers are converted locally to frequency.

## Boundaries

This slice does not test musical patches, chords, envelopes, drums, or MIDI
input. It is only a basic waveform audibility probe for the Core Gray speaker
bridge.

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

- serial logs show `amy: tone_on` and `amy: tone_off` for each waveform;
- `dropped` remains zero or rare;
- `blocked` remains stable or grows slowly;
- triangle or square should be judged against sine at the same pitch;
- if triangle or square is much clearer, future built-in speaker sounds should
  favor waveforms with harmonics over pure sine fundamentals;
- if note 72 is much clearer than note 60, future built-in speaker patches
  should avoid relying on low fundamentals.

## Hardware Observation

The waveform probe gave a clearer boundary:

- all C5 tones were clear;
- C4 sine was audible but weak;
- C4 triangle was better than sine;
- C4 square was excessive.

This points to a practical Core Gray built-in speaker rule: keep simple AMY
tones in the C5 range when possible, use triangle as the safer lower-register
waveform, and avoid raw square as a default voice because it is too aggressive
on this speaker path.
