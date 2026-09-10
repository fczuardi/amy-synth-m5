#include <Arduino.h>
#include <M5Unified.h>

#include "AmyM5SpeakerBridge.h"
#include "AmySynthVoice.h"
#include "BleMidiInput.h"
#include "InstrumentEventSink.h"

namespace {
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint32_t AUDIO_RELEASE_TAIL_MS = 700;
constexpr uint8_t AMY_SYNTH_ID = 1;
constexpr uint8_t AMY_MONO_VOICES = 1;
constexpr uint8_t INITIAL_JUNO_PATCH = 19;

AmyM5SpeakerBridge amyBridge;
AmySynthVoice synthVoice;
BleMidiInput bleMidiInput;

uint32_t lastStatusLogAtMs = 0;
uint32_t audioAwakeUntilMs = 0;
bool audioBridgeAwake = false;

float normalizedVelocity(uint8_t velocity) {
  return static_cast<float>(velocity) / 127.0f;
}

void wakeAudioBridge(uint32_t tailMs = AUDIO_RELEASE_TAIL_MS) {
  audioAwakeUntilMs = millis() + tailMs;
  audioBridgeAwake = true;
}

class AmyInstrumentSink : public InstrumentEventSink {
 public:
  void onNoteEvent(const NoteEvent& event) override {
    if (event.type == NoteEventType::NoteOn && event.velocity > 0) {
      if (synthVoice.noteActive()) {
        synthVoice.stopActiveNote();
      }

      wakeAudioBridge();
      synthVoice.noteOn(event.note, normalizedVelocity(event.velocity));
      Serial.printf("amy_midi: note_on channel=%u note=%u velocity=%u patch=%u\n",
                    event.channel,
                    event.note,
                    event.velocity,
                    synthVoice.patchNumber());
      return;
    }

    synthVoice.noteOff(event.note);
    wakeAudioBridge();
    Serial.printf("amy_midi: note_off channel=%u note=%u velocity=%u patch=%u\n",
                  event.channel,
                  event.note,
                  event.velocity,
                  synthVoice.patchNumber());
  }

  void onPitchBendEvent(const PitchBendEvent& event) override {
    synthVoice.setPitchBend(event.value);
    wakeAudioBridge();
    Serial.printf("amy_midi: pitch_bend channel=%u value=%d\n",
                  event.channel,
                  event.value);
  }

  void onDisconnected() override {
    panic("ble_disconnect");
  }

  void panic(const char* reason) {
    synthVoice.setPitchBend(0);
    synthVoice.stopActiveNote();
    wakeAudioBridge();
    Serial.printf("amy_midi: panic reason=%s\n", reason);
  }
};

class SerialBleDiagnostics : public BleMidiInputDiagnosticSink {
 public:
  void onBleMidiAdvertising(const char* deviceName) override {
    Serial.printf("ble_midi: advertising device=%s\n", deviceName);
  }

  void onBleMidiConnected() override {
    Serial.println("ble_midi: connected");
  }

  void onBleMidiDisconnected() override {
    Serial.println("ble_midi: disconnected");
  }

  void onBleMidiActiveSensing(uint32_t count, uint32_t activityAtMs) override {
    Serial.printf("ble_midi: active_sensing count=%lu activity_ms=%lu\n",
                  static_cast<unsigned long>(count),
                  static_cast<unsigned long>(activityAtMs));
  }

  void onBleMidiNoteEvent(
      const NoteEvent& event,
      uint32_t activityAtMs) override {
    Serial.printf("ble_midi: note_event type=%u channel=%u note=%u velocity=%u activity_ms=%lu\n",
                  static_cast<unsigned>(event.type),
                  event.channel,
                  event.note,
                  event.velocity,
                  static_cast<unsigned long>(activityAtMs));
  }

  void onBleMidiControlChange(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue,
      uint32_t activityAtMs) override {
    Serial.printf("ble_midi: control_change channel=%u controller=%u value=%u activity_ms=%lu\n",
                  channel,
                  controllerNumber,
                  controllerValue,
                  static_cast<unsigned long>(activityAtMs));
  }

  void onBleMidiPitchBend(
      uint8_t channel,
      int bendValue,
      uint32_t activityAtMs) override {
    Serial.printf("ble_midi: pitch_bend channel=%u value=%d activity_ms=%lu\n",
                  channel,
                  bendValue,
                  static_cast<unsigned long>(activityAtMs));
  }

  void onBleMidiDroppedEvents(uint32_t droppedEventCount) override {
    Serial.printf("ble_midi: dropped_events count=%lu\n",
                  static_cast<unsigned long>(droppedEventCount));
  }
};

AmyInstrumentSink amyInstrumentSink;
SerialBleDiagnostics bleDiagnostics;

void configureM5StackCoreGray() {
  auto config = M5.config();
  config.internal_spk = true;
  config.internal_mic = false;
  M5.begin(config);
}

void drawStaticScreen() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(96);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("BLE MIDI");
  M5.Display.println("AMY");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.println("Device: M5 Gray AMY");
  M5.Display.printf("Patch: %u\n", INITIAL_JUNO_PATCH);
  M5.Display.println("BtnA: panic");
}

void logStatus() {
  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs < STATUS_LOG_INTERVAL_MS) {
    return;
  }

  lastStatusLogAtMs = nowMs;
  Serial.printf("status: uptime_ms=%lu amy_started=%s rendered=%lu queued=%lu dropped=%lu blocked=%lu speaker_queue=%u patch=%u pitch_bend=%d note_active=%s\n",
                static_cast<unsigned long>(nowMs),
                amyBridge.amyStarted() ? "true" : "false",
                static_cast<unsigned long>(amyBridge.renderedBlockCount()),
                static_cast<unsigned long>(amyBridge.queuedBufferCount()),
                static_cast<unsigned long>(amyBridge.droppedBufferCount()),
                static_cast<unsigned long>(amyBridge.queueBlockedCount()),
                static_cast<unsigned>(amyBridge.speakerQueueDepth()),
                synthVoice.patchNumber(),
                synthVoice.pitchBend(),
                synthVoice.noteActive() ? "true" : "false");
}

void updateAudioBridge() {
  const bool shouldRender =
      synthVoice.noteActive() ||
      static_cast<int32_t>(millis() - audioAwakeUntilMs) < 0;

  if (shouldRender) {
    amyBridge.update();
    audioBridgeAwake = true;
    return;
  }

  if (audioBridgeAwake) {
    amyBridge.stopOutput();
    audioBridgeAwake = false;
    Serial.println("amy_audio: idle");
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);

  configureM5StackCoreGray();
  drawStaticScreen();

  Serial.println();
  Serial.println("BLE MIDI AMY Core Gray probe");
  Serial.printf("amy: sample_rate=%u block_size=%u channels=%u patch=%u voices=%u\n",
                AMY_SAMPLE_RATE,
                AMY_BLOCK_SIZE,
                AMY_NCHANS,
                INITIAL_JUNO_PATCH,
                AMY_MONO_VOICES);

  amyBridge.begin();
  synthVoice.begin(AMY_SYNTH_ID, AMY_MONO_VOICES, INITIAL_JUNO_PATCH);

  bleMidiInput.setInstrumentEventSink(&amyInstrumentSink);
  bleMidiInput.setDiagnosticSink(&bleDiagnostics);
  bleMidiInput.begin();
}

void loop() {
  M5.update();
  bleMidiInput.update();
  updateAudioBridge();

  if (M5.BtnA.wasPressed()) {
    amyInstrumentSink.panic("button_a");
  }

  logStatus();
}
