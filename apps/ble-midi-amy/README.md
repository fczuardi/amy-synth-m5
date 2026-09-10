# BLE MIDI AMY

Minimal BLE MIDI AMY synth probe for the M5Stack Core Gray.

The app consumes the existing `ble-midi-input` package and routes shared
`NoteEvent` and `PitchBendEvent` objects to AMY through the local
`amy-probe-runtime` helpers.

Build:

```bash
pio run
```

Upload:

```bash
pio run --target upload
```

The device advertises as `M5 Gray AMY`.
