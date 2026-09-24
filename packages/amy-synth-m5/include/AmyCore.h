#pragma once

// AMY's public C API must retain C linkage when consumed by our C++ adapters.
// AMY-Arduino.h provides this wrapper for Arduino builds; keep the portable
// core include independent of that Arduino-only connector.
extern "C" {
#include <amy.h>
}
