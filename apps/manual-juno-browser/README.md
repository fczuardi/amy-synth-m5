# Manual Juno Browser

Manual AMY/Juno patch browser for the M5Stack Core Gray.

Controls:

- button A plays MIDI note 72 while held;
- button B selects the previous patch when no note is active;
- button C selects the next patch when no note is active;
- while A is held, B bends down and C bends up;
- releasing B/C returns pitch bend to center.

Build:

```bash
pio run
```

Upload:

```bash
pio run --target upload
```
