# BLE MIDI AMY Dual Channel

This is a hardware probe for the two-patch configuration of
`AmyM5MonophonicSynth`. Raw MIDI channels 0 and 1, corresponding to physical
channels 1 and 2, select patches 19 and 24 for one global monophonic AMY slot.

Build and upload from this directory:

```sh
pio run -e m5stack-core-gray --target upload
pio device monitor -e m5stack-core-gray
```

The probe is successful when channel selection changes the patch on the next
note, the instrument remains globally monophonic, and disconnect or Button A
silences the note.
