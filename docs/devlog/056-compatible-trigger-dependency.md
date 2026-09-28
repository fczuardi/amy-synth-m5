# 056 — Accept compatible trigger package updates

## Goal

Allow applications to adopt new backward-compatible `step-trigger` features
without creating two installed copies or forcing an unnecessary AMY API
change.

## Change

The `step-trigger` dependency changes from exactly `0.1.0` to `>=0.1.0` and
the AMY package advances to `0.3.3`. AMY still consumes only the original
`StepLevel` and `TriggerEventSink` contract; the new pattern cursor is an
application concern.

## Validation

Calculator and electronic-drummer firmware builds both resolve the local
`step-trigger@0.2.0` alongside `amy-synth-m5@0.3.3` and compile successfully.
