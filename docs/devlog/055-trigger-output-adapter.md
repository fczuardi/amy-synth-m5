# 055 — Deduplicate AMY trigger output plumbing

## Goal

Remove repeated trigger-level conversion, audio wake-up, and AMY note-on code
from the Calculator and electronic drummer consumers.

## Boundary

`AmyTriggerOutput` accepts a MIDI note, semantic `StepLevel`, and optional
per-voice `TriggerVelocityCurve`. `AmyTriggerSink` additionally owns four
configurable lane mappings and implements `TriggerEventSink` directly.

The default curve preserves the validated weak/normal/strong values
0.45/0.70/1.00. A caller may attenuate a dominant sound without changing the
authored pattern levels. Pattern storage, timing, sound selection, and musical
style remain outside the AMY package.

Calculator uses the output form because sound assignments vary per pattern;
fixed-preset compositions can configure the sink and emit patterns directly.

## Validation

```text
pio test -d packages/amy-synth-m5 -e native
pio pkg pack packages/amy-synth-m5 --output /tmp
```

Consumer firmware builds provide the hardware-facing integration check.

Calculator hardware playback passed with unchanged triggering and distinct
weak, normal, and strong accents.
