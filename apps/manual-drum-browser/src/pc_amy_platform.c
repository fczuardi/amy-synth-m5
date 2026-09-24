#include <stddef.h>
#include <stdint.h>

#include <amy.h>

// Native SDL uses AMY's synchronous rendering API and sends the resulting
// block to M5Unified. These hooks satisfy the platform seam without starting
// AMY's independent miniaudio output thread.
void amy_platform_init(void) {}
void amy_platform_deinit(void) {}
void amy_update_tasks(void) {}

int16_t* amy_render_audio(void) {
  return amy_simple_fill_buffer();
}

size_t amy_i2s_write(const uint8_t*, size_t nbytes) {
  return nbytes;
}
