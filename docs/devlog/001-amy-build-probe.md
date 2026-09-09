# AMY Build Probe

## Goal

Start the AMY audio-engine exploration outside the umbrella showcase
repository. The first question is intentionally small: can a minimal
PlatformIO Arduino firmware for the M5Stack Core Gray include, link, and start
AMY without involving BLE MIDI, shared contracts, or the existing monophonic
instrument packages?

## Design

This probe keeps AMY in `AMY_AUDIO_IS_NONE` mode. That avoids guessing how AMY's
default I2S path should connect to the Core Gray internal speaker and keeps the
slice focused on dependency and toolchain compatibility.

The firmware still initializes M5Unified so the target matches the intended
hardware. On boot it writes a short status screen and serial log, starts AMY
with default synths enabled but no audio output, then calls `amy_update()` from
the Arduino loop.

AMY 1.2.108 uses the newer ESP-IDF I2S standard driver headers in its ESP32
source. The official `platformio/espressif32@7.0.1` package still provides the
Arduino-ESP32 2.x-era framework used by the existing baseline projects, so this
probe uses the community `pioarduino/platform-espressif32` stable package to
test against an Arduino-ESP32 3.x-compatible toolchain.

## Boundaries

This slice does not attempt to:

- produce sound;
- receive BLE MIDI;
- consume `InstrumentEventSink`;
- adapt `NoteEvent` or `PitchBendEvent`;
- define a reusable PCM or speaker abstraction.

Those are later questions if this build probe succeeds.

## Verification

Command-line target:

```bash
pio run
```

Hardware target, after a successful build:

```bash
pio run --target upload
```

Expected hardware observation for this slice is only that the firmware boots,
the display shows `AMY probe`, and the serial monitor reports that AMY started
with no audio output. Audible sound is explicitly out of scope.

## Results

The first build attempt used the existing project baseline toolchain:

```ini
platform = platformio/espressif32@7.0.1
```

That installed Arduino-ESP32 `3.20017.241212+sha.dcc1105b` and compiled far
enough to show the real mismatch:

```text
.pio/libdeps/m5stack-core-gray/amy/src/i2s.c:59:10: fatal error: driver/i2s_std.h: No such file or directory
```

Switching only the PlatformIO platform to `pioarduino/platform-espressif32`
stable pulled Arduino-ESP32 `3.3.11`, ESP-IDF libraries
`5.5.5+sha.b774170ff46`, and cleared the missing I2S driver header.

The successful command was:

```bash
pio run
```

Build result:

```text
RAM:   12.8% (used 41872 bytes from 327680 bytes)
Flash: 27.9% (used 877043 bytes from 3145728 bytes)
Total image size: 892763 bytes
```

This validates the narrow dependency/build question. The next risk is runtime
hardware behavior: whether the firmware boots reliably on the Core Gray with
this newer Arduino toolchain before any sound-output path is adapted.
