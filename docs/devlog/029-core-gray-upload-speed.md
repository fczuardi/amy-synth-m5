# Core Gray upload speed

The manual Juno browser targets the original M5Stack Core Gray. Its upload
speed now matches the other validated Core Gray environments: `460800` baud.

At `1500000` baud, esptool could start the stub flasher but the chip stopped
responding while verifying the flash connection after the baud-rate change.
The lower speed is the established reliable setting for this board in the
workspace. Plus2 environments remain at `1500000`.

This is a PlatformIO configuration change only; no hardware validation was
performed in this slice.
