# Juno application CI build

## Goal

Protect the hardware-critical 16-channel Juno firmware in CI, not only the
package archive and its small isolated consumer.

## Design

The normal CI workflow and the tag-triggered package publication workflow now
build `apps/ble-midi-amy-juno` after the isolated package consumer. The
application is the integration path validated on the Core Gray and currently
leaves only 101 bytes of IRAM free. PlatformIO's linker summary records IRAM,
DRAM, and flash usage directly in the job log on every build. The publication
step therefore cannot run if the critical application no longer links.

The normal `m5stack-core-gray` environment continues to consume sibling
workspace checkouts for local development. A derived
`m5stack-core-gray-ci` environment instead consumes the published
`monophonic-instrument@0.1.5` and `ble-midi-input@0.4.0` packages while
explicitly consuming the local `packages/amy-synth-m5` directory under review.
Those package manifests resolve `firmware-contracts@0.2.0` from its pinned Git
revision because that header-only package is not published separately in the
PlatformIO Registry. This makes the application build work from an isolated
checkout without weakening the local multi-repo workflow.

The cache key includes the Juno application configuration and source. A hard
numeric memory budget is deliberately deferred: the pioarduino platform URL
still follows its `stable` release, so a numeric comparison would not yet have
a fully pinned toolchain baseline. Link failure still protects the physical
IRAM limit, and every successful CI run records the detailed memory table.

The first CI run exposed that the current pioarduino `stable` platform requires
PlatformIO Core 6.2.0. Both the normal CI and package-publication workflows now
pin Core 6.2.0; their former 6.1.19 pin failed dependency resolution before the
firmware could build. This is a toolchain compatibility update, not a firmware
behavior change.

## Verification

The CI-equivalent local command is:

```sh
PLATFORMIO_CORE_DIR=.platformio-home \
  pio run -d apps/ble-midi-amy-juno -e m5stack-core-gray-ci
```

The expected dependency graph uses published external packages and local
`amy-synth-m5@0.3.0`. The linked firmware must retain the measured Core Gray
baseline of 130,971 bytes IRAM, 62,912 bytes DRAM, and 1,231,563 bytes total
image size with the current toolchain and local workspace dependencies. The
isolated CI dependency graph currently produces the same IRAM and DRAM values
and a 1,230,999-byte total image.
