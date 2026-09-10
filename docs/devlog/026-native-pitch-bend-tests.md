# Native Pitch Bend Tests

## Goal

Add a small, low-risk native test slice to `amy-synth-m5` without forcing new
interfaces around AMY or M5Unified.

## Design

The easiest useful seam is AMY pitch-bend conversion. `AmyRuntime` previously
kept the clamp and octave conversion helpers private. That made the behavior
hard to test without invoking AMY's runtime event queue.

This slice extracts the pure mapping to `AmyPitchBend`:

- clamp MIDI pitch bend to `-8192..8191`;
- map the centered MIDI range to AMY octave units using AMY's MIDI handler
  formula, `value / (6 * 8192)`;
- let `AmyRuntime` call the tested helper instead of owning a private copy.

The package now has a minimal native PlatformIO test environment with
`test_build_src = false`, so native tests can cover pure code without compiling
AMY or M5Unified.

## Verification

```sh
pio test -e native
```

Result:

- 5 test cases passed for `AmyPitchBend`.

The firmware builds still need to pass after the helper extraction because
`AmyRuntime` uses it in the actual AMY event path.

## Limits

This does not test `AmyMonophonicInstrumentSink`, `AmyRuntime` event emission,
or the Core Gray speaker bridge. Those still require either hardware validation
or a future fakeable boundary. The useful win here is that the pitch-bend math
is now covered without introducing premature abstraction.
