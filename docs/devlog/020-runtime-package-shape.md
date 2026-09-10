# Runtime Package Shape

## Goal

Make the existing `amy-probe-runtime` boundary easier to evaluate as a future
package candidate without changing firmware behavior.

## Design

The runtime library now has its own README describing the responsibilities that
have emerged from the hardware slices:

- `AmyM5SpeakerBridge` owns AMY PCM rendering into the Core Gray speaker path;
- `AmyAudioActivityGate` owns speaker output lifecycle and idle shutdown;
- `AmyRuntime` owns AMY-wide controls;
- `AmySynthSlot` owns a thin AMY synth-slot control wrapper.

The audio bridge is the strongest extraction candidate because it has a clear
hardware responsibility and two real local consumers: the manual Juno browser
and the BLE MIDI AMY app. It also captures the empirical Core Gray findings:
paced AMY rendering, stereo-to-mono mixing, small fixed buffers, conservative
M5Unified queue depth, output gain, and stopping the speaker channel at rest.

`AmyRuntime` and `AmySynthSlot` remain internal and provisional. They are
useful, but their final shape depends on future decisions around AMY synth
slots, multi-voice patches, global pitch bend, and where MIDI performance
policy belongs.

## Non-Goals

This slice does not export a package, rename classes, change audio behavior, or
move code into the umbrella repository. It only sharpens the local boundary so
the next extraction decision has written criteria.

## Verification Target

```bash
cd apps/manual-juno-browser
pio run

cd ../ble-midi-amy
pio run
```
