#include <SDL.h>
#include <M5GFX.h>
#include <M5Unified.h>

#if defined(SDL_h_)
void setup(void);
void loop(void);

__attribute__((weak)) int user_func(bool* running) {
  setup();
  do {
    loop();
  } while (*running);
  // Panel_sdl destroys the SDL window after this thread returns. Stop the
  // M5Unified speaker task first so it does not keep using SDL during teardown.
  // Discard queued samples as well; otherwise Speaker.end() waits for the
  // entire desktop audio queue to drain after a burst of test triggers.
  SDL_ClearQueuedAudio(1);
  M5.Speaker.end();
  return 0;
}

int main(int, char**) {
  // M5GFX supplies the standard arrow-key mappings. Labeled A/B/C keys are
  // captured separately by PcSdlInput after SDL initialization.
  return lgfx::Panel_sdl::main(user_func, 8);
}
#endif
