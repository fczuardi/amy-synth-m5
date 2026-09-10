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
- Keep runnable firmware under `apps/` and exported package candidates under
  `packages/`.
- Treat this repository as a package host/monorepo. The repository root is not
  a PlatformIO package; each exported package must live under
  `packages/<name>/` with its own `library.json`. Consume sibling packages by
  concrete package directory or packed archive, not by Git repository root.

## Boundaries

- Do not import BLE MIDI, shared contracts, or monophonic-instrument packages
  until AMY builds and produces local sound independently.
- Do not add umbrella showcases here; graduate a composition back to
  `embedded-music-experiments` only after the engine boundary is understood.
- Do not design generic audio abstractions before a real AMY output path makes
  the repeated shape visible.
- `packages/amy-synth-m5/` is package-shaped and CI-consumed, but still young.
  Keep API changes small and validated by at least the package consumer plus
  local apps.
