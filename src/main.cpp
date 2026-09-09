#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint32_t NOTE_ON_INTERVAL_MS = 2500;
constexpr uint32_t NOTE_DURATION_MS = 900;
constexpr float TEST_FREQUENCIES_HZ[] = {220.0f, 330.0f, 440.0f, 660.0f};
constexpr uint8_t AUDIO_CHANNEL = 0;

uint32_t lastStatusLogAtMs = 0;
uint32_t lastNoteOnAtMs = 0;
uint32_t currentNoteStartedAtMs = 0;
size_t nextFrequencyIndex = 0;
bool noteActive = false;
bool muted = false;
float activeFrequencyHz = 0.0f;

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
  M5.Display.println("Mode: M5 speaker tone");
  M5.Display.println("AMY: bypassed");
  M5.Display.printf("Frequency: %.2f Hz\n", activeFrequencyHz);
  M5.Display.print("Muted: ");
  M5.Display.println(muted ? "yes" : "no");
  M5.Display.print("Note: ");
  M5.Display.println(noteActive ? "on" : "off");
  M5.Display.print("State: ");
  M5.Display.println(stateLabel);
}

void configureSpeaker() {
  auto speakerConfig = M5.Speaker.config();
  // Keep the Core Gray's default mono DAC speaker path and output rate.
  M5.Speaker.config(speakerConfig);
  M5.Speaker.begin();
  M5.Speaker.setVolume(255);
  M5.Speaker.setAllChannelVolume(255);
}

void toneOn(float frequencyHz) {
  if (muted) {
    return;
  }

  M5.Speaker.tone(frequencyHz, NOTE_DURATION_MS, AUDIO_CHANNEL, true);
  activeFrequencyHz = frequencyHz;
  noteActive = true;
  currentNoteStartedAtMs = millis();
  Serial.printf("m5speaker: tone_on frequency_hz=%.2f\n", frequencyHz);
}

void toneOff() {
  M5.Speaker.stop(AUDIO_CHANNEL);
  activeFrequencyHz = 0.0f;
  noteActive = false;
  Serial.println("m5speaker: tone_off");
}

void toggleMute() {
  muted = !muted;
  if (muted) {
    toneOff();
  } else {
    lastNoteOnAtMs = millis() - NOTE_ON_INTERVAL_MS;
  }

  Serial.printf("m5speaker: muted=%s\n", muted ? "true" : "false");
  drawScreen(muted ? "muted" : "running");
}

void updateTestNotes() {
  const uint32_t nowMs = millis();

  if (noteActive && nowMs - currentNoteStartedAtMs >= NOTE_DURATION_MS) {
    toneOff();
  }

  if (!noteActive && nowMs - lastNoteOnAtMs >= NOTE_ON_INTERVAL_MS) {
    lastNoteOnAtMs = nowMs;
    toneOn(TEST_FREQUENCIES_HZ[nextFrequencyIndex]);
    nextFrequencyIndex =
        (nextFrequencyIndex + 1) %
        (sizeof(TEST_FREQUENCIES_HZ) / sizeof(TEST_FREQUENCIES_HZ[0]));
  }
}
}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);

  auto config = M5.config();
  M5.begin(config);
  M5.Display.setRotation(1);

  Serial.println();
  Serial.println("Core Gray M5Unified speaker tone baseline");
  Serial.printf("board_id=%d\n", static_cast<int>(M5.getBoard()));
  Serial.println("amy: bypassed for speaker baseline");

  configureSpeaker();
  drawScreen("speaker ready");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    toggleMute();
  }

  updateTestNotes();

  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs >= STATUS_LOG_INTERVAL_MS) {
    lastStatusLogAtMs = nowMs;
    Serial.printf("status: uptime_ms=%lu amy_bypassed=true muted=%s speaker_queue=%u note_active=%s frequency_hz=%.2f\n",
                  static_cast<unsigned long>(nowMs),
                  muted ? "true" : "false",
                  static_cast<unsigned>(M5.Speaker.isPlaying(AUDIO_CHANNEL)),
                  noteActive ? "true" : "false",
                  activeFrequencyHz);
    drawScreen("running");
  }
}
