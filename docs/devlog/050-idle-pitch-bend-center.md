# Clear Pitch Bend at center while idle

## Goal

Prevent a bend used on one note from leaking into the next note when the
performer releases the key before releasing the physical Pitch Bend strip.

## Hardware finding

The first active-channel policy rejected every Pitch Bend event while idle.
This sequence exposed the missing cleanup path:

1. hold a note;
2. hold Pitch Bend up;
3. release the note;
4. release the strip to center;
5. play another note.

Note Off stopped the voice but did not reset AMY's global bend. The center
event then arrived while idle and was rejected, so the next note could start
with the previous positive bend.

Hardware audition also found a click when the strip returns abruptly to center
while a note remains held. That event is accepted by both the old and refined
policies, so the click is separate from idle filtering. It is recorded as a
future AMY pitch-transition/smoothing investigation rather than folded into
this state-policy fix.

## Design

The refined policy distinguishes cleanup from new idle performance state:

- while a note is active, accept Pitch Bend only from its channel;
- normalize values in the `-128..128` center dead zone to zero;
- while idle, reject every value outside that center dead zone;
- while idle, accept a normalized center value from any supported channel.

Accepting idle center lets a spring-loaded wheel or touch strip clear the
global AMY state. Rejecting idle non-center values still prevents one channel
from preparing an unexpected bend for a later note on another channel.

The first implementation required exact zero. A serial trace of the failing
hardware sequence showed that the Arturia strip settled at value 64 after
descending from values around 8,062. Exact-zero matching therefore rejected
the real center event. The `-128..128` dead zone matches the established
monophonic instrument policy and sends a true zero to AMY rather than retaining
the controller's small center offset.

## Verification

Native policy tests cover dead-zone normalization, idle non-center rejection,
idle center acceptance, active-channel acceptance, and other-channel rejection.
The complete Core Gray Juno firmware remains the integration build target.

The focused hardware check repeats the five-step sequence above. The final
note must start at center pitch. Returning the strip to center while the first
note remains held may still click; that separate observation is not claimed as
fixed here.
