#include <Arduino.h>
#include <M5Unified.h>
#include <AMY-Arduino.h>

#include <cstring>

#include "AmyM5MonophonicSynth.h"
#include "BleMidiInput.h"

namespace {
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint8_t FIRST_SYNTH_ID = 1;
constexpr uint8_t MONO_VOICES = 1;
constexpr uint8_t FIRST_PATCH = 19;
constexpr uint8_t SECOND_PATCH = 24;

AmyM5MonophonicSynth amySynth;
BleMidiInput bleMidiInput;
uint32_t lastStatusLogAtMs = 0;

class AmyModWheelMappingSink : public InstrumentEventSink {
 public:
  explicit AmyModWheelMappingSink(AmyM5MonophonicSynth& synth)
      : synth_(synth) {
  }

  void onNoteEvent(const NoteEvent& event) override {
    synth_.onNoteEvent(event);
  }

  void onPitchBendEvent(const PitchBendEvent& event) override {
    synth_.onPitchBendEvent(event);
  }

  void onControlChangeEvent(const ControlChangeEvent& event) override {
    if (event.channel != AmyM5MonophonicSynth::FIRST_MIDI_CHANNEL ||
        event.controller != 1) {
      return;
    }

    uint8_t rawMessage[3] = {
        static_cast<uint8_t>(0xB0 | event.channel),
        event.controller,
        event.value,
    };
    midi_msg_handler(rawMessage, sizeof(rawMessage), 0, 0);
  }

  void onDisconnected() override {
    synth_.onDisconnected();
  }

 private:
  AmyM5MonophonicSynth& synth_;
};

AmyModWheelMappingSink amyModWheelMappingSink(amySynth);

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
    Serial.printf("ble_midi: pitch_bend channel=%u value=%d activity_ms=%lu\n",
                  channel,
                  bendValue,
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
  M5.Display.println("AMY DUAL");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.println("Ch 1: patch 19");
  M5.Display.println("Ch 2: patch 24");
  M5.Display.println("BtnA: panic");
}

void logStatus() {
  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs < STATUS_LOG_INTERVAL_MS) {
    return;
  }
  lastStatusLogAtMs = nowMs;
  Serial.printf("status: uptime_ms=%lu note_active=%s selected_patch=%u patch1=%u patch2=%u pitch_bend=%d\n",
                static_cast<unsigned long>(nowMs),
                amySynth.noteActive() ? "true" : "false",
                amySynth.patchNumber(),
                amySynth.patchNumberForChannel(AmyM5MonophonicSynth::FIRST_MIDI_CHANNEL),
                amySynth.patchNumberForChannel(AmyM5MonophonicSynth::SECOND_MIDI_CHANNEL),
                amySynth.pitchBend());
}

void configureChannelOneModWheelMapping() {
  // AMY MIDI channels are one-based. v2 is the first Juno sounding oscillator
  // relative to each voice, while the empty fields preserve other coefficients.
  constexpr char MAPPING[] = "v2f,,,,,%vZ";
  const int configured = midi_store_mapping(
      1, MIDI_MAP_TYPE_CC, 1, 0, 0.0f, 0.1f, 0.0f, MAPPING,
      std::strlen(MAPPING));
  Serial.printf("amy: mod_wheel_mapping channel=1 cc=1 status=%d\n", configured);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
  configureM5StackCoreGray();
  drawStaticScreen();

  Serial.println();
  Serial.println("BLE MIDI AMY dual-channel Core Gray probe");
  Serial.printf("amy: sample_rate=%u block_size=%u first_patch=%u second_patch=%u voices=%u\n",
                AMY_SAMPLE_RATE,
                AMY_BLOCK_SIZE,
                FIRST_PATCH,
                SECOND_PATCH,
                MONO_VOICES);

  amySynth.begin(FIRST_SYNTH_ID, MONO_VOICES, FIRST_PATCH, SECOND_PATCH);
  configureChannelOneModWheelMapping();
  bleMidiInput.setInstrumentEventSink(&amyModWheelMappingSink);
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

  logStatus();
}
