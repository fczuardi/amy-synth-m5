# BLE MIDI AMY

Minimal BLE MIDI AMY synth probe for the M5Stack Core Gray.

The app consumes the existing `ble-midi-input` package and routes shared
`NoteEvent` and `PitchBendEvent` objects to AMY through the local
`amy-synth-m5` package candidate. The AMY package reuses
`MonophonicNotePriority` from `monophonic-instrument` for held-note priority.

Build:

```bash
pio run
```

Upload:

```bash
pio run --target upload
```

The firmware sets the BLE device name to `M5 Gray AMY` and logs it at boot.
Some Android BLE MIDI apps may display the target as `Bluetooth MIDI` instead.
