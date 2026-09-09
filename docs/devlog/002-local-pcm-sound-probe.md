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

The serial log also showed a UART driver error. The probe does not use MIDI
input yet, so the firmware now explicitly keeps AMY MIDI disabled and clears
the ESP32 default `midi_uart` value to `-1`. That keeps this slice focused on
local PCM output instead of starting an unused UART MIDI path.

That was not sufficient by itself. AMY's ESP32 no-multithread update path calls
`esp_poll_midi()` unconditionally from `amy_update_tasks()`, even when MIDI mode
is disabled. The probe now leaves AMY rendering in multithread mode while keeping
multicore rendering off. That avoids the unconditional UART poll and may also
make audio block production steadier for the M5Unified queue.

The follow-up monitor run confirmed that the UART error flood stopped. The
status logs showed AMY still running, notes cycling, the speaker queue holding
at two blocks, and no dropped audio blocks:

```text
status: uptime_ms=3000 amy_started=true rendered=373 queued=373 dropped=0 speaker_queue=2 note_active=true
status: uptime_ms=10000 amy_started=true rendered=1525 queued=1525 dropped=0 speaker_queue=2 note_active=true
status: uptime_ms=30000 amy_started=true rendered=4822 queued=4822 dropped=0 speaker_queue=2 note_active=false
```

That makes the multithread setting the current known-good path for this PCM
bridge probe.

The first sound-quality adjustment stopped overriding the Core Gray speaker
configuration as stereo 44.1 kHz output. M5Unified's detected Core Gray default
uses the built-in mono DAC speaker path, so the probe now preserves that board
configuration and only feeds AMY's stereo PCM block as stereo input to
`playRaw()`. On hardware this sounded less noisy and more recognizable, though
still quieter than desired. The next conservative loudness step raises the
M5Unified master volume to 255 while leaving the board-specific speaker
magnification unchanged.

The volume increase did not make an audible difference. The notes were
recognizable, but there were still clicks and interference. The next hypothesis
is that queuing each 256-frame AMY block as a separate `playRaw()` clip creates
audible boundaries. The bridge now aggregates eight AMY blocks into each
M5Unified raw audio chunk, reducing the clip-boundary rate from about 172 per
second to about 21 per second.

To separate bridge quality from patch complexity, the probe also switched from
AMY's default synth/Juno-style patch routing to a single raw oscillator. The
firmware now disables default synths and cycles one `SINE` oscillator through
220, 330, 440, and 660 Hz. A clean result should sound like plain test tones,
not a musical patch.

The sine probe made the pitch sequence clearer, but the speaker still produced
radio-like interference or artifacts while the tones were otherwise
recognizable. That means the next isolation step is to remove AMY from the
runtime path entirely. The current bench firmware starts only M5Unified's
speaker support and cycles the same frequencies with `M5.Speaker.tone()`. If
this direct speaker baseline is clean, the artifact is likely in the AMY PCM to
`playRaw()` bridge. If this baseline is also noisy, the limit is more likely in
the Core Gray built-in speaker/DAC/power path or the local bench setup.

Because these repeated test tones are intrusive during bench work, the baseline
also adds a simple mute control. Pressing Button A toggles sound on and off,
stops any tone currently playing, and logs the mute state over serial. Unmuting
allows the next tone to start immediately so the same firmware can remain on
the device while listening conditions change.

The direct M5Unified speaker baseline was clean on hardware: the radio-like
interference heard during the AMY sine probe disappeared. That makes the Core
Gray built-in speaker path usable and shifts the next investigation back to the
AMY PCM bridge, especially how `amy_update()` blocks are handed to
`M5.Speaker.playRaw()`.
