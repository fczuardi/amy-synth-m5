#include <Arduino.h>
#include <M5Unified.h>

#include "AmyAudioActivityGate.h"
#include "AmyM5SpeakerBridge.h"
#include "AmyRuntime.h"
#include "AmySynthSlot.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint8_t AUDITION_NOTE = 72;

constexpr uint8_t AMY_SYNTH_ID = 1;
constexpr uint8_t AMY_MONO_VOICES = 1;
constexpr uint8_t JUNO_PATCH_COUNT = 128;
constexpr float AMY_NOTE_VELOCITY = 1.0f;
constexpr int16_t PITCH_BEND_CENTER = 0;
constexpr int16_t PITCH_BEND_DOWN = -8192;
constexpr int16_t PITCH_BEND_UP = 8191;

uint32_t lastStatusLogAtMs = 0;

bool muted = false;
bool auditionNoteActive = false;
uint8_t activeMidiNote = AUDITION_NOTE;
int16_t currentPitchBend = PITCH_BEND_CENTER;
size_t activePatchIndex = 0;
AmyM5SpeakerBridge amyBridge;
AmyAudioActivityGate audioGate(amyBridge);
AmyRuntime amyRuntime;
AmySynthSlot synthSlot;

void drawScreen(const char* stateLabel) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("AMY probe");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.println("Mode: AMY stream");
  M5.Display.println("Patch: Juno audition");
  M5.Display.print("Muted: ");
  M5.Display.println(muted ? "yes" : "no");
  M5.Display.print("State: ");
  M5.Display.println(stateLabel);
}

uint8_t activePatchNumber() {
  return static_cast<uint8_t>(activePatchIndex);
}

void configureActivePatch() {
  const uint8_t patchNumber = activePatchNumber();
  synthSlot.setPatch(patchNumber);

  Serial.printf("amy: patch_configured synth=%u patch=%u voices=%u\n",
                AMY_SYNTH_ID,
                patchNumber,
                AMY_MONO_VOICES);
}

void startAuditionNote() {
  audioGate.wake();
  synthSlot.noteOn(AUDITION_NOTE, AMY_NOTE_VELOCITY);
  activeMidiNote = AUDITION_NOTE;
  auditionNoteActive = true;
  Serial.printf("amy: note_on patch=%u midi_note=%u\n",
                synthSlot.patchNumber(),
                AUDITION_NOTE);
}

void stopAuditionNote() {
  const uint8_t releasedNote = activeMidiNote;
  amyRuntime.setGlobalPitchBend(PITCH_BEND_CENTER);
  currentPitchBend = PITCH_BEND_CENTER;
  synthSlot.noteOff(releasedNote);
  auditionNoteActive = false;
  audioGate.wake();
  Serial.printf("amy: note_off patch=%u midi_note=%u\n",
                synthSlot.patchNumber(),
                releasedNote);
}

void setPitchBend(int16_t value) {
  if (currentPitchBend == value) {
    return;
  }

  amyRuntime.setGlobalPitchBend(value);
  currentPitchBend = value;
  audioGate.wake();
  Serial.printf("amy: pitch_bend value=%d\n", value);
}

void setPatchIndex(size_t patchIndex) {
  if (auditionNoteActive) {
    stopAuditionNote();
  }

  activePatchIndex = patchIndex;
  configureActivePatch();
}

void nextPatch() {
  setPatchIndex((activePatchIndex + 1) % JUNO_PATCH_COUNT);
}

void previousPatch() {
  setPatchIndex((activePatchIndex + JUNO_PATCH_COUNT - 1) % JUNO_PATCH_COUNT);
}

void updateButtons() {
  if (M5.BtnA.wasPressed() && !auditionNoteActive) {
    startAuditionNote();
  }
  if (M5.BtnA.wasReleased() && auditionNoteActive) {
    stopAuditionNote();
  }
  if (auditionNoteActive && M5.BtnB.wasPressed()) {
    setPitchBend(PITCH_BEND_DOWN);
  }
  if (auditionNoteActive && M5.BtnC.wasPressed()) {
    setPitchBend(PITCH_BEND_UP);
  }
  if (auditionNoteActive &&
      ((M5.BtnB.wasReleased() && currentPitchBend == PITCH_BEND_DOWN) ||
       (M5.BtnC.wasReleased() && currentPitchBend == PITCH_BEND_UP))) {
    setPitchBend(PITCH_BEND_CENTER);
  }
  if (!auditionNoteActive && M5.BtnB.wasPressed()) {
    previousPatch();
  }
  if (!auditionNoteActive && M5.BtnC.wasPressed()) {
    nextPatch();
  }
}

void logStatus() {
  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs < STATUS_LOG_INTERVAL_MS) {
    return;
  }

  lastStatusLogAtMs = nowMs;
  Serial.printf("status: uptime_ms=%lu muted=%s rendered=%lu queued=%lu dropped=%lu blocked=%lu speaker_queue=%u patch=%u pitch_bend=%d note_active=%s\n",
                static_cast<unsigned long>(nowMs),
                muted ? "true" : "false",
                static_cast<unsigned long>(amyBridge.renderedBlockCount()),
                static_cast<unsigned long>(amyBridge.queuedBufferCount()),
                static_cast<unsigned long>(amyBridge.droppedBufferCount()),
                static_cast<unsigned long>(amyBridge.queueBlockedCount()),
                static_cast<unsigned>(amyBridge.speakerQueueDepth()),
                activePatchNumber(),
                amyRuntime.globalPitchBend(),
                auditionNoteActive ? "true" : "false");
}

void updateAudioBridge() {
  if (muted) {
    if (audioGate.awake()) {
      audioGate.forceIdle();
    }
    return;
  }

  const bool wasAwake = audioGate.awake();
  const bool isAwake = audioGate.update(auditionNoteActive);
  if (wasAwake && !isAwake) {
    Serial.println("amy: audio_idle");
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
  Serial.println("AMY Core Gray streaming probe");
  Serial.printf("amy: sample_rate=%u block_size=%u channels=%u\n",
                AMY_SAMPLE_RATE,
                AMY_BLOCK_SIZE,
                AMY_NCHANS);
  Serial.printf("amy: juno_patch_browser patches=%u voices=%u note=%u\n",
                JUNO_PATCH_COUNT,
                AMY_MONO_VOICES,
                AUDITION_NOTE);

  drawScreen("starting AMY");
  amyBridge.begin();
  amyRuntime.begin(AMY_SYNTH_ID);
  synthSlot.begin(AMY_SYNTH_ID, AMY_MONO_VOICES, activePatchNumber());
  Serial.printf("amy: patch_configured synth=%u patch=%u voices=%u\n",
                AMY_SYNTH_ID,
                activePatchNumber(),
                AMY_MONO_VOICES);
  drawScreen("running");
}

void loop() {
  M5.update();
  updateButtons();
  updateAudioBridge();
  logStatus();
}
