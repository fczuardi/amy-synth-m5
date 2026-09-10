# BLE MIDI Scan Response Name

## Goal

Make the AMY BLE MIDI app consume the `ble-midi-input` fix that exposes the
configured device name through NimBLE scan-response data.

## Problem

A Linux `btmon` scan of the Core Gray running the AMY firmware showed the BLE
MIDI service UUID in the primary advertisement, but an empty scan response:

```text
Address: 80:7D:3A:DC:A2:AA
Advertising Data:
  Flags
  128-bit Service UUIDs

Scan response:
  Data length: 0
```

That means the device was not actually advertising `M5 Gray AMY`, even though
the firmware configured and logged that name. `bluetoothctl` showing
`M5 Gray Speaker` was BlueZ cached display state for the same ESP32 MAC, not
current raw advertising data.

## Design

The fix belongs to the `ble-midi-input` package. BLE MIDI's 128-bit service
UUID leaves too little room for typical device names in the 31-byte legacy
advertising packet, so `ble-midi-input@0.1.4` enables NimBLE scan response data
before setting the name.

This app now pins `ble-midi-input` to `midi-receiver` commit `c40e71bf` so the
AMY probe builds against that fixed package revision instead of an older local
PlatformIO dependency cache.

## Verification Target

```bash
cd apps/ble-midi-amy
pio run
pio run --target upload
```

Hardware follow-up should scan with `btmon` and confirm the Core Gray scan
response includes:

```text
Name (complete): M5 Gray AMY
```
