# Fallback Patch Selection

## Goal

Ensure a cross-channel monophonic fallback resumes with the patch associated
with the returning note's channel.

## Design

The shared `MonophonicNotePriority` policy now includes the MIDI channel in
each `MonophonicNoteAction`. `AmyMonophonicInstrumentSink` exposes the start
action before applying it. `AmyM5MonophonicSynth` uses that action to select the
channel's configured patch before the sink starts the note.

The facade treats configured control values as global performance state. It
remembers one latest value per mapped controller and applies that value to the
currently active channel's patch whenever a note starts, including after a
patch reload. This matches the single physical modulation strip without adding
state to the generic MIDI or monophonic contracts. The replay uses AMY's current
event clock; timestamp zero would schedule the restored CC before the patch-load
event, allowing patch initialization to reset the coefficient again.

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
channel-1 note uses channel 1's patch. This was subsequently validated on the
Core Gray; the restored modulation was also audibly retained after the
cross-channel fallback.

## Deferred follow-up

The current implementation deliberately keeps one latest CC1 value for the
facade's single physical modulation strip. A future slice may experiment with
remembering CC1 independently for each configured MIDI channel and restoring
that value when the channel becomes active. This would be state in the AMY
facade, not a claim that AMY provides per-channel CC1: the underlying AMY
control remains global. The experiment is deferred until the current global
behavior is considered complete.
