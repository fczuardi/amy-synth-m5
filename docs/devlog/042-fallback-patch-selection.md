# Fallback Patch Selection

## Goal

Ensure a cross-channel monophonic fallback resumes with the patch associated
with the returning note's channel.

## Design

The shared `MonophonicNotePriority` policy now includes the MIDI channel in
each `MonophonicNoteAction`. `AmyMonophonicInstrumentSink` exposes the start
action before applying it. `AmyM5MonophonicSynth` uses that action to select the
channel's configured patch before the sink starts the note.

This keeps responsibilities explicit: the policy chooses the note identity,
the AMY facade maps that identity to a patch, and the sink translates the
already-selected action into slot commands. Frequency-oriented consumers can
carry the same channel information without needing to interpret it.

## Verification

The monophonic package's 51 native tests passed. The dual-channel AMY firmware
build passed against the local `monophonic-instrument@0.1.4` checkout. IRAM
usage remains 129,811 bytes, with 1,261 bytes free.

The new hardware check still required is: play a note on channel 1, replace it
with a note on channel 2, release channel 2, and confirm that the returning
channel-1 note uses channel 1's patch. No new hardware verification is claimed
here.
