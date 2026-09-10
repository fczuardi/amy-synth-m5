#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

#include "amy_synth_voice.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint8_t AUDITION_NOTE = 72;

constexpr uint8_t AMY_SYNTH_ID = 1;
constexpr uint8_t AMY_MONO_VOICES = 1;
constexpr uint8_t JUNO_PATCH_COUNT = 128;
constexpr float AMY_NOTE_VELOCITY = 1.0f;

constexpr uint8_t AUDIO_CHANNEL = 0;
constexpr int32_t OUTPUT_GAIN = 2;

constexpr uint32_t AMY_BLOCK_INTERVAL_US =
    (AMY_BLOCK_SIZE * 1000000UL) / AMY_SAMPLE_RATE;
constexpr uint8_t MAX_CATCH_UP_BLOCKS_PER_LOOP = 3;
constexpr size_t STREAM_BUFFER_COUNT = 3;
constexpr size_t AMY_BLOCKS_PER_STREAM_BUFFER = 3;
constexpr size_t STREAM_BUFFER_SAMPLES =
    AMY_BLOCK_SIZE * AMY_BLOCKS_PER_STREAM_BUFFER;

int16_t streamBuffers[STREAM_BUFFER_COUNT][STREAM_BUFFER_SAMPLES];

uint32_t lastStatusLogAtMs = 0;
uint32_t lastAmyRenderAtUs = 0;

uint32_t renderedBlockCount = 0;
uint32_t queuedBufferCount = 0;
uint32_t droppedBufferCount = 0;
uint32_t queueBlockedCount = 0;

size_t activeBufferIndex = 0;
size_t activeBufferSamples = 0;

bool amyStarted = false;
bool muted = false;
size_t activePatchIndex = 0;
AmySynthVoice synthVoice;

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

void configureSpeaker() {
  auto speakerConfig = M5.Speaker.config();
  M5.Speaker.config(speakerConfig);
  M5.Speaker.begin();
  M5.Speaker.setVolume(255);
  M5.Speaker.setAllChannelVolume(255);
}

void startAmyForPcm() {
  amy_config_t amyConfig = amy_default_config();
  amyConfig.audio = AMY_AUDIO_IS_NONE;
  amyConfig.midi = AMY_MIDI_IS_NONE;
  amyConfig.midi_uart = -1;
  amyConfig.midi_in = -1;
  amyConfig.midi_out = -1;
  amyConfig.features.startup_bleep = 0;
  amyConfig.features.default_synths = 0;
  amyConfig.features.audio_in = 0;
  amyConfig.platform.multithread = 1;
  amyConfig.platform.multicore = 0;

  amy_start(amyConfig);
  amyStarted = true;
  Serial.printf("amy: juno_patch_browser patches=%u voices=%u note=%u\n",
                JUNO_PATCH_COUNT,
                AMY_MONO_VOICES,
                AUDITION_NOTE);
}

uint8_t activePatchNumber() {
  return static_cast<uint8_t>(activePatchIndex);
}

void configureActivePatch() {
  const uint8_t patchNumber = activePatchNumber();
  synthVoice.setPatch(patchNumber);

  Serial.printf("amy: patch_configured synth=%u patch=%u voices=%u\n",
                AMY_SYNTH_ID,
                patchNumber,
                AMY_MONO_VOICES);
}

void startAuditionNote() {
  synthVoice.noteOn(AUDITION_NOTE, AMY_NOTE_VELOCITY);
  Serial.printf("amy: note_on patch=%u midi_note=%u\n",
                synthVoice.patchNumber(),
                AUDITION_NOTE);
}

void stopAuditionNote() {
  const uint8_t releasedNote = synthVoice.activeMidiNote();
  synthVoice.stopActiveNote();
  Serial.printf("amy: note_off patch=%u midi_note=%u\n",
                synthVoice.patchNumber(),
                releasedNote);
}

void setPatchIndex(size_t patchIndex) {
  if (synthVoice.noteActive()) {
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
  if (M5.BtnA.wasPressed() && !synthVoice.noteActive()) {
    startAuditionNote();
  }
  if (M5.BtnA.wasReleased() && synthVoice.noteActive()) {
    stopAuditionNote();
  }
  if (M5.BtnB.wasPressed()) {
    previousPatch();
  }
  if (M5.BtnC.wasPressed()) {
    nextPatch();
  }
}

bool queueActiveBuffer() {
  if (activeBufferSamples == 0) {
    return true;
  }

  if (M5.Speaker.isPlaying(AUDIO_CHANNEL) >= 2) {
    return false;
  }

  const bool queued = M5.Speaker.playRaw(streamBuffers[activeBufferIndex],
                                         activeBufferSamples,
                                         AMY_SAMPLE_RATE,
                                         false,
                                         1,
                                         AUDIO_CHANNEL,
                                         false);
  if (!queued) {
    droppedBufferCount++;
    return false;
  }

  queuedBufferCount++;
  activeBufferIndex = (activeBufferIndex + 1) % STREAM_BUFFER_COUNT;
  activeBufferSamples = 0;
  return true;
}

int16_t mixFrameToMono(int16_t* samples, size_t frame) {
  const int32_t left = samples[frame * AMY_NCHANS];
  const int32_t right = samples[(frame * AMY_NCHANS) + 1];
  int32_t mono = ((left + right) / 2) * OUTPUT_GAIN;

  if (mono > INT16_MAX) {
    return INT16_MAX;
  }
  if (mono < INT16_MIN) {
    return INT16_MIN;
  }
  return static_cast<int16_t>(mono);
}

void appendAmyBlock(int16_t* samples) {
  if (samples == nullptr || muted) {
    return;
  }

  int16_t* output = streamBuffers[activeBufferIndex] + activeBufferSamples;
  for (size_t frame = 0; frame < AMY_BLOCK_SIZE; ++frame) {
    output[frame] = mixFrameToMono(samples, frame);
  }

  activeBufferSamples += AMY_BLOCK_SIZE;
  renderedBlockCount++;

  if (activeBufferSamples >= STREAM_BUFFER_SAMPLES && !queueActiveBuffer()) {
    droppedBufferCount++;
    activeBufferSamples = 0;
  }
}

bool canRenderNextAmyBlock() {
  if (activeBufferSamples + AMY_BLOCK_SIZE < STREAM_BUFFER_SAMPLES) {
    return true;
  }

  return M5.Speaker.isPlaying(AUDIO_CHANNEL) < 2;
}

void serviceAmyAudio() {
  const uint32_t nowUs = micros();
  uint8_t catchUpBlocks = 0;

  while (amyStarted && !muted &&
         nowUs - lastAmyRenderAtUs >= AMY_BLOCK_INTERVAL_US &&
         catchUpBlocks < MAX_CATCH_UP_BLOCKS_PER_LOOP) {
    if (!canRenderNextAmyBlock()) {
      queueBlockedCount++;
      lastAmyRenderAtUs += AMY_BLOCK_INTERVAL_US;
      break;
    }

    appendAmyBlock(amy_update());
    lastAmyRenderAtUs += AMY_BLOCK_INTERVAL_US;
    catchUpBlocks++;
  }
}

void logStatus() {
  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs < STATUS_LOG_INTERVAL_MS) {
    return;
  }

  lastStatusLogAtMs = nowMs;
  Serial.printf("status: uptime_ms=%lu muted=%s rendered=%lu queued=%lu dropped=%lu blocked=%lu speaker_queue=%u patch=%u note_active=%s\n",
                static_cast<unsigned long>(nowMs),
                muted ? "true" : "false",
                static_cast<unsigned long>(renderedBlockCount),
                static_cast<unsigned long>(queuedBufferCount),
                static_cast<unsigned long>(droppedBufferCount),
                static_cast<unsigned long>(queueBlockedCount),
                static_cast<unsigned>(M5.Speaker.isPlaying(AUDIO_CHANNEL)),
                activePatchNumber(),
                synthVoice.noteActive() ? "true" : "false");
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
  Serial.printf("stream: buffers=%u samples_per_buffer=%u gain=%ldx\n",
                static_cast<unsigned>(STREAM_BUFFER_COUNT),
                static_cast<unsigned>(STREAM_BUFFER_SAMPLES),
                static_cast<long>(OUTPUT_GAIN));

  configureSpeaker();
  drawScreen("starting AMY");
  startAmyForPcm();
  synthVoice.begin(AMY_SYNTH_ID, AMY_MONO_VOICES, activePatchNumber());
  Serial.printf("amy: patch_configured synth=%u patch=%u voices=%u\n",
                AMY_SYNTH_ID,
                activePatchNumber(),
                AMY_MONO_VOICES);
  lastAmyRenderAtUs = micros();
  drawScreen("running");
}

void loop() {
  M5.update();
  updateButtons();
  serviceAmyAudio();
  logStatus();
}
