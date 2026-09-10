# Monophonic Release Asset Dependency Limit

## Goal

Replace the temporary checkout-based CI dependency with a reproducible remote
package artifact.

## Problem

`amy-synth-m5` needs `MonophonicNotePriority` from the sibling
`monophonic-instrument` package. The sibling repository is a package
host/monorepo, so its Git repository root is not a PlatformIO package.

Three artifacts are easy to confuse:

- GitHub Packages: not a generic PlatformIO/C++ package registry for this
  project;
- GitHub's automatic release source archive: contains the whole monorepo and is
  not the package boundary;
- a custom GitHub Release asset created by `pio pkg pack`: contains the concrete
  PlatformIO package directory and is installable by PlatformIO from a URL.

The third artifact is the correct one.

## Release Asset

The monophonic package release was published from:

```text
monophonic-instrument tag: monophonic-instrument-v0.1.2
asset: monophonic-instrument-0.1.2.tar.gz
```

Asset URL:

```text
https://github.com/fczuardi/monophonic-instrument/releases/download/monophonic-instrument-v0.1.2/monophonic-instrument-0.1.2.tar.gz
```

The tag version matches `packages/monophonic-instrument/library.json`.

## Discovery

The intended manifest dependency was:

```json
{
  "name": "monophonic-instrument",
  "version": "https://github.com/fczuardi/monophonic-instrument/releases/download/monophonic-instrument-v0.1.2/monophonic-instrument-0.1.2.tar.gz"
}
```

That URL is the right artifact, but it cannot currently live in
`packages/amy-synth-m5/library.json`: PlatformIO 6.1.19 validates dependency
`version` strings with a 100-character maximum, and this exact GitHub Release
asset URL is longer.

A short release asset alias without an extension was also tested. Local
`file://` installation worked because PlatformIO detects archive contents, but
the remote GitHub URL failed: PlatformIO treats GitHub URLs without `.zip`,
`.tar.gz`, or `.tar.xz` suffixes as Git repositories.

Therefore the desired transitive path remains pending:

```text
amy-synth-m5 package
-> manifest dependency
-> remote monophonic package tarball
```

The CI was kept on the hermetic checkout-based path from slice 027 so it stays
green while preserving the monorepo boundaries.

## Verification

```sh
pio pkg pack packages/amy-synth-m5 --output /tmp
pio run -s  # in ci/consumers/amy-synth-m5
pio run -d apps/manual-juno-browser
pio run -d apps/ble-midi-amy
```

The package pack initially failed with the long release URL in `library.json`.
The no-extension alias installed from `file://` but failed from GitHub as a
remote dependency. After restoring the slice-027 checkout dependency, the AMY
package packed and the isolated consumer built again. Both existing firmware
apps also built with the local explicit monophonic package dependency.

## Limits

No new hardware verification was performed in this slice. It only investigates
package distribution. The hardware-validated behavior remains the prior Core
Gray BLE MIDI AMY validation: notes, velocity, pitch bend, held-note fallback,
panic, reconnect, and idle gating.
