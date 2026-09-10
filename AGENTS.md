# Repository Guidelines

This repository is an experimental sibling of `embedded-music-experiments`.
It is allowed to be rougher than the umbrella showcase repository because its
job is to expose audio-engine dependency, build, memory, and hardware-output
risks before any architecture is promoted.

## Workflow

- Use `jj` for normal version control work.
- Keep changes small and hardware- or command-line-testable.
- Record one narrative chapter per slice under `docs/devlog/`.
- Prefer PlatformIO with the Arduino framework for ESP32/M5Stack probes.
- Keep `core_dir` pointed at `/home/fcz/dev/m5stick/.platformio-home`.
- Keep runnable firmware under `apps/` so the repository root remains available
  for future `packages/` if an AMY boundary matures.

## Boundaries

- Do not import BLE MIDI, shared contracts, or monophonic-instrument packages
  until AMY builds and produces local sound independently.
- Do not add umbrella showcases here; graduate a composition back to
  `embedded-music-experiments` only after the engine boundary is understood.
- Do not design generic audio abstractions before a real AMY output path makes
  the repeated shape visible.
- `lib/amy-probe-runtime/` is internal to this repo. Do not treat it as a
  package contract until more than one real app proves a stable boundary.
