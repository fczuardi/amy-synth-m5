# Local PCM Sound Probe

## Goal

The first slice proved that AMY can build, upload, boot, and stay alive on the
M5Stack Core Gray when using the newer `pioarduino/platform-espressif32`
toolchain. This slice asks the next small question: can AMY-rendered audio
samples reach the Core Gray built-in speaker without introducing BLE MIDI,
shared contracts, or a new audio abstraction?

## Design

AMY remains in `AMY_AUDIO_IS_NONE` mode. That means AMY renders PCM blocks but
does not own the ESP32 I2S peripheral. The firmware calls `amy_update()`, copies
the returned signed 16-bit stereo sample block into a small rotating buffer, and
queues that block into `M5.Speaker.playRaw()`.

This deliberately keeps board-specific speaker output with M5Unified. AMY's
direct I2S examples target ordinary I2S DAC wiring and boards such as AMYboard;
the Core Gray built-in speaker path is already handled by M5Unified, so the
first local sound attempt uses that proven path instead of guessing pins.

The test sound is deterministic. The firmware configures AMY synth 1 with a
Juno-style patch, then cycles through four MIDI notes. The screen and serial log
report rendered, queued, and dropped audio block counts so underrun behavior is
observable even before judging the sound by ear.

## Boundaries

This slice does not attempt to:

- receive BLE MIDI;
- consume shared `NoteEvent` or `PitchBendEvent` contracts;
- support arbitrary patches or controls;
- prove polyphony;
- optimize latency or buffering;
- define a reusable AMY adapter.

The only target is local sound from AMY PCM through the built-in Core Gray
speaker.

## Verification

Command-line build:

```bash
pio run
```

Build result:

```text
RAM:   14.0% (used 46008 bytes from 327680 bytes)
Flash: 28.2% (used 886523 bytes from 3145728 bytes)
Total image size: 902511 bytes
```

Hardware target:

```bash
pio run --target upload
pio device monitor
```

Expected observation: the display shows `local PCM sound`, serial logs show
rendered and queued blocks increasing, and the Core Gray speaker plays a
short repeating note pattern. Dropped blocks should ideally remain at zero or
low enough that the result is still recognizable.

Upload result:

```text
Auto-detected: /dev/ttyUSB0
Chip type: ESP32-D0WDQ6 (revision v1.0)
Hash of data verified.
Hard resetting via RTS pin.
```

The upload passed, but after reset the serial device was no longer visible as
`/dev/ttyUSB0` or `/dev/ttyACM*` from this shell, so monitor output and audible
hardware observations remain open for the bench check.

## Hardware Observation

The hardware check produced audible output from the Core Gray built-in speaker
after the upload. The result was not yet a clear tune, but it was definitely
AMY-driven sound: short blocks of sound separated by gaps, matching the current
simple block-queueing approach.

That answers this slice's main question. AMY-rendered PCM can reach the Core
Gray speaker through M5Unified. The next risk is continuity and clarity: the
current loop only queues a block when the M5Unified channel has space, so gaps,
drops, or queue starvation are expected. A follow-up slice should focus on
steady audio buffering before adding BLE, contracts, or musical controls.
