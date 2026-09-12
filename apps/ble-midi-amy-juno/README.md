# BLE MIDI AMY Juno

This application exercises the fixed 16-channel configuration of
`AmyM5MonophonicSynth`. Each zero-based MIDI channel selects one visible Juno
patch from the application table for one globally monophonic AMY slot. It is
selectable by physical MIDI channels 1 through 16, not a 16-voice or
multitimbral instrument.

Build and upload from this directory:

```sh
pio run -e m5stack-core-gray --target upload
pio device monitor -e m5stack-core-gray
```

The application is successful when channel selection changes the patch on the
next note, cross-channel fallback preserves the held-note identity and patch,
the Juno modulation mapping responds to CC1, and disconnect or Button A
silences the note.
