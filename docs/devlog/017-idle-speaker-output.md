# Idle Speaker Output

## Goal

After the BLE MIDI app played its first note, the Core Gray speaker path kept a
residual hum or hiss even after the note ended. Before the first note, the app
was silent. This slice checks whether the noise comes from keeping the
M5Unified speaker output active with silent PCM buffers.

## Design

`AmyM5SpeakerBridge` now exposes `stopOutput()`. It clears the partially filled
stream buffer and asks M5Unified to stop the AMY audio channel.

The bridge still only owns the PCM-to-speaker mechanics. The apps own the
performance policy:

- wake the bridge before Note On, Note Off, Pitch Bend, or panic;
- keep rendering through a short `700 ms` release tail;
- stop the speaker channel once the note is inactive and the tail has elapsed.

This keeps AMY release tails audible while avoiding endless silent buffers that
leave the Core Gray DAC/amplifier path open at rest.

## Verification

Command-line builds:

```bash
cd apps/ble-midi-amy
pio run

cd ../manual-juno-browser
pio run
```

Both builds passed.

## Hardware Observation

The BLE MIDI app was uploaded to the Core Gray. After playing and releasing a
note, the residual hum/hiss disappeared after the idle transition, confirming
that continuously streaming silent PCM was the cause.

The same hardware session also tested a real USB MIDI keyboard connected to the
Android device through USB OTG and bridged through SynthBridge. Note velocity
and pitch bend both reached the AMY app correctly.
