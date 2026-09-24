# Native SDL compatibility follows the M5 stack

## Goal

Replace the first experimental native shim's independent `std::chrono` clock
with the SDL compatibility primitives already provided by M5GFX.

## Finding

M5's own SDL examples use an `ArduinoSDL.h` compatibility header. M5GFX
already exposes SDL-backed `lgfx::millis`, `lgfx::micros`, `lgfx::delay`,
`lgfx::delayMicroseconds`, and GPIO helpers, so a project should delegate to
those instead of maintaining a second timing origin.

## Change

The drum browser now keeps a small `Arduino.h` forwarding header for AMY's
legacy include, while the implementation lives in `ArduinoSDL.h` and reuses
M5GFX timing. The serial shim remains project-local because the browser's
diagnostic logs are not part of Arduino's timing contract.

## Verification

```text
pio run -e native-sdl -d apps/manual-drum-browser
```

The compatibility layer and all C++ sources compile successfully. The native
AMY C source list is now added explicitly by `pc_amy_sources.py`, and the
`AmyCore.h` wrapper preserves C linkage for the C API. The browser links and
builds successfully with:

```text
just build-drums-pc
```

Running with `SDL_AUDIODRIVER=dummy` also completes its smoke test. A normal
desktop-audio run can abort while being forcibly terminated by `timeout`, so
interactive shutdown and SDL audio-driver behavior remain a follow-up test.
The SDL entrypoint now explicitly ends the M5Unified speaker task before the
window teardown, avoiding an audio thread using SDL after `Panel_sdl` closes it.

## Runtime diagnosis

The first native audio attempt appeared to freeze after a few triggers. A GDB
thread dump showed the application thread spinning in AMY's
`libminiaudio-audio.c` implementation of `amy_render_audio`, waiting forever
for `last_audio_buffer` while AMY was configured with `AMY_AUDIO_IS_NONE`.
The native target now uses AMY's synchronous `amy_simple_fill_buffer()` API and
small platform hooks instead of starting the unrelated miniaudio backend.

## Publication boundary

The hardware-facing `amy-synth-m5` package can publish the portable `AmyCore.h`
boundary and continue to declare AMY as a normal remote PlatformIO dependency.
The SDL-specific source integration is not part of the package release yet:
`pc_amy_sources.py` currently points at a local `.pio/libdeps` checkout. Before
advertising native support, make that dependency discovery reproducible in a
clean installation and validate the package without relying on the hardware
environment's cache. Until then, SDL remains a documented local showcase
target rather than a published package contract.
