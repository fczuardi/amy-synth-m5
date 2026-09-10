# BLE MIDI AMY App

## Goal

The manual browser validated AMY note and pitch-bend control locally, but the
next architectural question needs real MIDI input. This slice adds a sibling
app that consumes the existing `ble-midi-input` package instead of duplicating
BLE MIDI parsing code.

## Design

The new app lives at:

```text
apps/ble-midi-amy/
```

The signal path is:

```text
BLE MIDI controller
-> ble-midi-input
-> NoteEvent / PitchBendEvent
-> local AmyInstrumentSink
-> AmySynthVoice
-> AMY renderer
-> AmyM5SpeakerBridge
-> M5.Speaker
```

The app advertises as `M5 Gray AMY`, uses AMY synth slot 1, and starts on Juno
patch 19. Program Change and Control Change are intentionally ignored for this
first slice.

The app also introduces an internal `amy-probe-runtime` library under `lib/`.
It is not an exported package. It only avoids duplicating the AMY synth wrapper
and the validated AMY-to-`M5.Speaker` PCM bridge between local apps.

`ble-midi-input` now defaults to BLE-MIDI's ESP32 NimBLE backend in the source
package, so this app does not carry an app-specific backend flag.

## Boundaries

This is still a probe app, not an umbrella showcase and not a package
promotion. The only external reusable boundary being exercised here is the
already-existing `ble-midi-input` package and its shared firmware contracts.

The local `AmyInstrumentSink` is deliberately app-local. It is the experiment
that will show whether a synth-performance sink is worth extracting later.

## Verification

Command-line builds:

```bash
cd apps/manual-juno-browser
pio run

cd ../ble-midi-amy
pio run
```

Both builds passed. The BLE app build installed `ble-midi-input` from the local
package path and compiled through the package's default NimBLE backend.

Hardware target:

```bash
cd apps/ble-midi-amy
pio run --target upload
pio device monitor
```

Expected observation:

- the device advertises as `M5 Gray AMY`;
- MIDI Note On/Off controls AMY patch 19;
- MIDI Pitch Bend bends the active AMY note;
- button A sends a local panic for the latest active note.
