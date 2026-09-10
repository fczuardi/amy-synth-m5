# AMY Pitch Bend Probe

## Goal

`AmySynthVoice` should represent synth performance controls rather than only
patch and note events. This slice adds pitch bend as the next small control and
validates whether AMY's event-level bend is useful on the Core Gray speaker
path.

## Design

AMY exposes `amy_event.pitch_bend` as a global pitch bend in octave units. Its
own MIDI handler maps signed MIDI pitch bend values like this:

```c
pitch_bend = midi_bend / (6.0f * 8192.0f)
```

That maps `-8192..8191` to roughly `-1/6..+1/6` octaves, or about +/-2
semitones. `AmySynthVoice::setPitchBend()` follows that same mapping so the
wrapper uses AMY's existing MIDI semantics.

The local controls keep the patch browser intact:

- with no note active, B and C still select previous/next patch;
- while A is held and a note is active, B bends down;
- while A is held and a note is active, C bends up;
- releasing B or C resets bend to center;
- releasing A also resets bend to center before note-off.

## Boundaries

AMY's current pitch bend is global, not truly per synth slot. The wrapper keeps
the method on `AmySynthVoice` because that is the performance boundary we are
testing, but future package design should remember that the underlying AMY
field may affect more than one active synth.

`AmySynthVoice` also remains a provisional name. It currently represents one
AMY synth slot, and that slot may be configured with more than one AMY voice.
Likewise, `activeMidiNote()` and `noteActive()` describe only the latest note
started through the wrapper. That is enough for the current monophonic browser,
but it is not complete polyphonic state. Before packaging, this boundary should
either drop active-note state entirely or grow enough state to support reliable
panic and multi-note control.

## Verification

Command-line build:

```bash
pio run
```

Hardware target:

```bash
pio run --target upload
pio device monitor
```

Expected observation:

- A still plays MIDI note 72 while held;
- B/C still browse patches when no note is active;
- B/C bend the held note down/up while A is held;
- releasing bend controls returns the pitch to center;
- status logs include `pitch_bend=`.

## Hardware Observation

Hardware testing on the M5Stack Core Gray confirmed that pitch bend is audible
in both directions:

- holding A still plays the selected patch on MIDI note 72;
- pressing B while holding A bends the note down;
- pressing C while holding A bends the note up;
- releasing the bend button returns the note to center;
- releasing A stops the note as before.

No new click, delay, or release artifact was reported during the bend test. The
test confirms that pitch bend belongs in the AMY performance-control direction,
while the global nature of AMY's current `pitch_bend` field remains a packaging
concern for any future multi-synth design.
