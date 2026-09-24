#pragma once

#include <M5GFX.h>

#include <cstdint>
#include <cstdio>

// M5GFX already provides the SDL-backed timing and GPIO compatibility layer.
// Reuse it instead of maintaining a second std::chrono implementation here.
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

using lgfx::delay;
using lgfx::delayMicroseconds;
using lgfx::micros;
using lgfx::millis;

#ifndef vTaskDelay
#define vTaskDelay delay
#endif

struct PcSerial {
  void begin(uint32_t) {}

  template <typename... Args>
  void printf(const char* format, Args... args) {
    std::printf(format, args...);
    std::fflush(stdout);
  }

  void println() {
    std::printf("\n");
    std::fflush(stdout);
  }

  void println(const char* value) {
    std::printf("%s\n", value);
    std::fflush(stdout);
  }
};

inline PcSerial Serial;
