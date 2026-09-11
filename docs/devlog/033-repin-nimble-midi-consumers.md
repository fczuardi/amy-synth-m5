# Repin AMY Consumers To NimBLE MIDI

## Finding

The AMY applications were still consuming `ble-midi-input` from commit
`c40e71bf`. That commit used the former `lathoub/BLE-MIDI` transport, even
though the receiver repository had already published `ble-midi-input@0.3.0`,
which implements the BLE MIDI service directly with NimBLE-Arduino.

This escaped the migration because the source repository and Showcase 3 were
updated, but the other application manifests were not audited afterward.

## Change

The one-channel AMY app and the two-channel probe now consume:

```ini
fcz2/ble-midi-input@0.3.0
```

The old `Active Sensing` diagnostic callback was removed from the AMY apps
because it is not part of the published `0.3.0` diagnostic interface.

## Verification

Both AMY applications built successfully with the published package after the
callback cleanup. The dual-channel build remains at 129811 bytes of IRAM and
uses 60896 bytes of DRAM. The existing one-channel build uses 129811 bytes of
IRAM and 60816 bytes of DRAM.

The umbrella showcases for the buzzer and Core Gray speaker also built
successfully after being repinned to `fcz2/ble-midi-input@0.3.0`. The umbrella
Showcase 3 build already used that version and continued to build successfully.

Physical AMY hardware validation of the NimBLE package repin is still pending.
