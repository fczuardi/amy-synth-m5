#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint32_t NOTE_ON_INTERVAL_MS = 2500;
constexpr uint32_t NOTE_DURATION_MS = 1400;

struct TestTone {
  const char* label;
  uint8_t wave;
  uint8_t midiNote;
};

constexpr TestTone TEST_TONES[] = {
    {"sine_c4", SINE, 60},
    {"triangle_c4", TRIANGLE, 60},
    {"square_c4", PULSE, 60},
    {"sine_c5", SINE, 72},
    {"triangle_c5", TRIANGLE, 72},
    {"square_c5", PULSE, 72},
};

constexpr float AMY_NOTE_VELOCITY = 1.0f;
constexpr uint8_t OSCILLATOR_BASE = 0;
constexpr float SQUARE_DUTY = 0.5f;

constexpr uint8_t AUDIO_CHANNEL = 0;
constexpr int32_t OUTPUT_GAIN = 6;
constexpr int32_t GATE_GAIN_SCALE = 32767;
constexpr size_t GATE_RAMP_SAMPLES = 1024;

constexpr uint32_t AMY_BLOCK_INTERVAL_US =
    (AMY_BLOCK_SIZE * 1000000UL) / AMY_SAMPLE_RATE;
constexpr uint8_t MAX_CATCH_UP_BLOCKS_PER_LOOP = 3;
constexpr size_t STREAM_BUFFER_COUNT = 3;
constexpr size_t AMY_BLOCKS_PER_STREAM_BUFFER = 3;
constexpr size_t STREAM_BUFFER_SAMPLES =
    AMY_BLOCK_SIZE * AMY_BLOCKS_PER_STREAM_BUFFER;

int16_t streamBuffers[STREAM_BUFFER_COUNT][STREAM_BUFFER_SAMPLES];

uint32_t lastStatusLogAtMs = 0;
uint32_t lastNoteOnAtMs = 0;
uint32_t currentNoteStartedAtMs = 0;
uint32_t lastAmyRenderAtUs = 0;

uint32_t renderedBlockCount = 0;
uint32_t queuedBufferCount = 0;
uint32_t droppedBufferCount = 0;
uint32_t queueBlockedCount = 0;

size_t activeBufferIndex = 0;
size_t activeBufferSamples = 0;

int32_t gateGain = 0;
int32_t gateTargetGain = 0;
int32_t gateStep = 0;

bool amyStarted = false;
bool noteActive = false;
bool muted = false;
size_t nextToneIndex = 0;
const TestTone* activeTone = nullptr;

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
  M5.Display.println("Patch: waveform probe");
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
  Serial.printf("amy: waveform_probe tones=%u\n",
                static_cast<unsigned>(sizeof(TEST_TONES) / sizeof(TEST_TONES[0])));
}

void setGateTarget(int32_t targetGain) {
  gateTargetGain = targetGain;

  const int32_t delta = gateTargetGain - gateGain;
  gateStep = delta / static_cast<int32_t>(GATE_RAMP_SAMPLES);
  if (gateStep == 0 && delta != 0) {
    gateStep = delta > 0 ? 1 : -1;
  }
}

float midiNoteToFrequency(uint8_t midiNote) {
  return 440.0f * powf(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
}

void noteOn(const TestTone& tone) {
  amy_event event = amy_default_event();
  event.osc = OSCILLATOR_BASE;
  event.wave = tone.wave;
  event.freq_coefs[COEF_CONST] = midiNoteToFrequency(tone.midiNote);
  if (tone.wave == PULSE) {
    event.duty_coefs[COEF_CONST] = SQUARE_DUTY;
  }
  event.velocity = AMY_NOTE_VELOCITY;
  amy_add_event(&event);

  Serial.printf("amy: tone_on label=%s wave=%u midi_note=%u\n",
                tone.label,
                static_cast<unsigned>(tone.wave),
                tone.midiNote);
}

void startTone(const TestTone& tone) {
  noteOn(tone);
  activeTone = &tone;
  setGateTarget(GATE_GAIN_SCALE);
  noteActive = true;
  currentNoteStartedAtMs = millis();
}

void noteOff() {
  amy_event event = amy_default_event();
  event.osc = OSCILLATOR_BASE;
  event.velocity = 0.0f;
  amy_add_event(&event);

  if (activeTone != nullptr) {
    Serial.printf("amy: tone_off label=%s midi_note=%u\n",
                  activeTone->label,
                  activeTone->midiNote);
  }
}

void stopTone() {
  noteOff();
  activeTone = nullptr;
  setGateTarget(0);
  noteActive = false;
}

void resetOutputState() {
  M5.Speaker.stop(AUDIO_CHANNEL);
  activeBufferSamples = 0;
  gateGain = 0;
  gateTargetGain = 0;
  gateStep = 0;
}

void toggleMute() {
  muted = !muted;
  if (muted) {
    if (noteActive) {
      stopTone();
    }
    resetOutputState();
  } else {
    lastNoteOnAtMs = millis() - NOTE_ON_INTERVAL_MS;
  }

  Serial.printf("amy: muted=%s\n", muted ? "true" : "false");
  drawScreen(muted ? "muted" : "running");
}

void updateTestNoteGate() {
  const uint32_t nowMs = millis();

  if (noteActive && nowMs - currentNoteStartedAtMs >= NOTE_DURATION_MS) {
    stopTone();
  }

  if (!noteActive && nowMs - lastNoteOnAtMs >= NOTE_ON_INTERVAL_MS) {
    lastNoteOnAtMs = nowMs;
    startTone(TEST_TONES[nextToneIndex]);
    nextToneIndex =
        (nextToneIndex + 1) % (sizeof(TEST_TONES) / sizeof(TEST_TONES[0]));
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

int32_t nextGateGain() {
  if (gateGain == gateTargetGain) {
    return gateGain;
  }

  const bool rising = gateStep > 0;
  gateGain += gateStep;
  if ((rising && gateGain > gateTargetGain) ||
      (!rising && gateGain < gateTargetGain)) {
    gateGain = gateTargetGain;
  }
  return gateGain;
}

int16_t mixFrameToMono(int16_t* samples, size_t frame) {
  const int32_t left = samples[frame * AMY_NCHANS];
  const int32_t right = samples[(frame * AMY_NCHANS) + 1];
  int32_t mono = ((left + right) / 2) * OUTPUT_GAIN;
  mono = (mono * nextGateGain()) / GATE_GAIN_SCALE;

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
  Serial.printf("status: uptime_ms=%lu muted=%s rendered=%lu queued=%lu dropped=%lu blocked=%lu speaker_queue=%u gate=%ld tone=%s note_active=%s\n",
                static_cast<unsigned long>(nowMs),
                muted ? "true" : "false",
                static_cast<unsigned long>(renderedBlockCount),
                static_cast<unsigned long>(queuedBufferCount),
                static_cast<unsigned long>(droppedBufferCount),
                static_cast<unsigned long>(queueBlockedCount),
                static_cast<unsigned>(M5.Speaker.isPlaying(AUDIO_CHANNEL)),
                static_cast<long>(gateGain),
                activeTone == nullptr ? "none" : activeTone->label,
                noteActive ? "true" : "false");
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
  lastAmyRenderAtUs = micros();
  drawScreen("running");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    toggleMute();
  }

  updateTestNoteGate();
  serviceAmyAudio();
  logStatus();
}
