# 024 - Rename Package to amy-synth-m5

## Goal

Remove `probe` from the package name before the first umbrella showcase
consumes it. The repository can still be exploratory, but the package artifact
should have a name that makes sense as a dependency.

## Changes

The package moved from:

```text
lib/amy-probe-runtime
```

to:

```text
packages/amy-synth-m5
```

The PlatformIO package name is now `amy-synth-m5`. Public C++ class names did
not change:

- `AmyRuntime`;
- `AmySynthSlot`;
- `AmyM5SpeakerBridge`;
- `AmyAudioActivityGate`.

The CI consumer moved from `ci/consumers/amy-probe-runtime` to
`ci/consumers/amy-synth-m5`, and the local apps now scan `../../packages`
through `lib_extra_dirs`.

The package manifest now points at the future repository URL:

```text
https://github.com/fczuardi/amy-synth-m5.git
```

The GitHub repository should be renamed after this slice is pushed so the
manifest URL and repository identity agree.

## Naming Rationale

`amy-synth-m5` describes the current scope better than `amy-probe-runtime`:

- AMY is the synth engine;
- synth communicates the musical domain;
- M5 marks the hardware family and avoids pretending this is a universal AMY
  wrapper;
- the name can still host future M5-specific AMY packages if this repo evolves
  into a small monorepo.

The package still contains two families with different portability:

- AMY control: runtime and synth slot commands;
- M5/Core Gray output: AMY PCM into M5Unified speaker output.

That split remains documented but not physically separated yet.

## Verification Target

```bash
pio pkg pack packages/amy-synth-m5 --output /tmp

pio run -d ci/consumers/amy-synth-m5

cd apps/manual-juno-browser
pio run

cd ../ble-midi-amy
pio run
```
