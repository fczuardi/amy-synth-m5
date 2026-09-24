"""Build the AMY C core for the native SDL target.

The upstream AMY package is published as an Arduino library, while its
README's Linux path compiles the core C sources directly. PlatformIO's native
compatibility filter therefore does not discover those sources automatically.
"""

from pathlib import Path

Import("env")


amy_src = Path(env.subst("$PROJECT_DIR/../../.pio/libdeps/m5stack-core-gray/amy/src"))

env.Append(
    CPPPATH=[str(amy_src)],
    CCFLAGS=["-DAMY_WAVETABLE", "-D_POSIX_THREADS"],
    LINKFLAGS=["-pthread", "-lm"],
)

# Match the source list used by AMY's native Makefile. The example programs
# and platform-specific entry points are not part of the embedded core.
env.BuildSources(
    env.subst("$BUILD_DIR/amy-core"),
    str(amy_src),
    [
        "+<algorithms.c>",
        "+<amy.c>",
        "+<envelope.c>",
        "+<parse.c>",
        "+<filters.c>",
        "+<oscillators.c>",
        "+<pcm.c>",
        "+<interp_partials.c>",
        "+<custom.c>",
        "+<delay.c>",
        "+<log2_exp2.c>",
        "+<patches.c>",
        "+<transfer.c>",
        "+<sequencer.c>",
        "+<instrument.c>",
        "+<amy_midi.c>",
        "+<api.c>",
        "+<midi_mappings.c>",
        "+<cv_trigger.c>",
    ],
)
