#include <Arduino.h>
#include <M5Unified.h>

#include "AmyM5MonophonicSynth.h"
#include "BleMidiInput.h"

namespace {
constexpr AmyM5MonophonicSynthConfiguration SYNTH_CONFIGURATION{
    .synthId = 1,
    .voiceCount = 1,
    .patches = {
        // Keep this table easy to edit while testing the Juno bank. The
        // selected entries are deliberately spread through the 128 presets.
        0, 9, 18, 24,
        32, 40, 49, 54,
        64, 73, 80, 89,
        96, 105, 114, 120,
    },
};

AmyM5MonophonicSynth amySynth;
BleMidiInput bleMidiInput;

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

  void onBleMidiNoteEvent(
      const NoteEvent& event,
      uint32_t activityAtMs) override {
    Serial.printf("ble_midi: note_event channel=%u type=%u note=%u velocity=%u activity_ms=%lu\n",
                  event.channel,
                  static_cast<unsigned>(event.type),
                  event.note,
                  event.velocity,
                  static_cast<unsigned long>(activityAtMs));
  }

  void onBleMidiPitchBend(
      uint8_t channel,
      int bendValue,
      uint32_t activityAtMs) override {
    // Pitch strips emit dense bursts of events. Logging every value can block
    // the firmware loop long enough to disturb audio, so keep this high-rate
    // diagnostic out of the performance app.
    (void) channel;
    (void) bendValue;
    (void) activityAtMs;
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

  void onBleMidiDroppedEvents(uint32_t droppedEventCount) override {
    Serial.printf("ble_midi: dropped_events count=%lu\n",
                  static_cast<unsigned long>(droppedEventCount));
  }
};

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
  M5.Display.println("AMY JUNO");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.println("Ch 1: patch 0");
  M5.Display.println("Ch 2: patch 9");
  M5.Display.println("Ch 3: patch 18");
  M5.Display.println("Ch 8: patch 54");
  M5.Display.println("BtnA: panic");
}

void configureJunoPerformanceModulation() {
  Serial.printf("amy: juno_modulation=%s\n",
                amySynth.configureJunoPerformanceModulation() ? "ok" : "error");
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
  configureM5StackCoreGray();
  drawStaticScreen();

  Serial.println();
  Serial.println("BLE MIDI AMY Juno Core Gray application");
  Serial.printf("amy: sample_rate=%u block_size=%u patches=%u voices=%u\n",
                AMY_SAMPLE_RATE,
                AMY_BLOCK_SIZE,
                AMY_M5_MIDI_CHANNEL_COUNT,
                SYNTH_CONFIGURATION.voiceCount);

  amySynth.begin(SYNTH_CONFIGURATION);
  configureJunoPerformanceModulation();
  bleMidiInput.setInstrumentEventSink(&amySynth);
  bleMidiInput.setDiagnosticSink(&bleDiagnostics);
  bleMidiInput.begin();
}

void loop() {
  M5.update();
  bleMidiInput.update();
  amySynth.update();

  if (M5.BtnA.wasPressed()) {
    amySynth.panic();
    Serial.println("amy_midi: panic reason=button_a");
  }
}
