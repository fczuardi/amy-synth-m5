# 030 Publish the AMY package

## Goal

Make the validated `amy-synth-m5` package available through the PlatformIO
Registry so external compositions can consume it without a sibling checkout.

## Distribution

The new GitHub Actions workflow runs for tags shaped like
`amy-synth-m5-v*`. It verifies that the tag version matches
`packages/amy-synth-m5/library.json`, runs native package tests, packs the
subpackage, builds its isolated consumer, and publishes only that subdirectory
with `pio pkg publish`.

The repository root remains a package host rather than a PlatformIO package.
The PlatformIO Registry is used for the package artifact; GitHub source archives
are not used because they contain the entire repository.

## Verification

The workflow is ready for the first tag `amy-synth-m5-v0.1.1`. The release still
requires the `PLATFORMIO_AUTH_TOKEN` repository secret and a successful GitHub
Actions run. No new hardware validation is part of this distribution slice.
