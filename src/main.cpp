#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;

uint32_t lastStatusLogAtMs = 0;
bool amyStarted = false;

void drawScreen(const char* stateLabel) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("AMY probe");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.printf("Board id: %d\n", static_cast<int>(M5.getBoard()));
  M5.Display.println("Target: Core Gray");
  M5.Display.println("Mode: build/runtime probe");
  M5.Display.println("Audio: none");
  M5.Display.print("AMY: ");
  M5.Display.println(amyStarted ? "started" : "not started");
  M5.Display.print("State: ");
  M5.Display.println(stateLabel);
}

void startAmyNoAudio() {
  amy_config_t amyConfig = amy_default_config();

  // First slice: prove the dependency links before adapting any M5 speaker path.
  amyConfig.audio = AMY_AUDIO_IS_NONE;
  amyConfig.features.startup_bleep = 0;
  amyConfig.features.default_synths = 1;

  amy_start(amyConfig);
  amyStarted = true;
}
}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);

  auto config = M5.config();
  M5.begin(config);
  M5.Display.setRotation(1);

  Serial.println();
  Serial.println("AMY Core Gray build probe");
  Serial.printf("board_id=%d\n", static_cast<int>(M5.getBoard()));

  drawScreen("starting AMY");
  startAmyNoAudio();

  Serial.println("amy: started with AMY_AUDIO_IS_NONE");
  drawScreen("AMY started");
}

void loop() {
  M5.update();

  if (amyStarted) {
    amy_update();
  }

  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs >= STATUS_LOG_INTERVAL_MS) {
    lastStatusLogAtMs = nowMs;
    Serial.printf("status: uptime_ms=%lu amy_started=%s\n",
                  static_cast<unsigned long>(nowMs),
                  amyStarted ? "true" : "false");
  }
}

