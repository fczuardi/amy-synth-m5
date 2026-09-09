#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint32_t NOTE_ON_INTERVAL_MS = 2500;
constexpr uint32_t NOTE_DURATION_MS = 1400;
constexpr uint8_t TEST_OSC_ID = 0;
constexpr float TEST_FREQUENCY_HZ = 440.0f;
constexpr float AMY_TEST_VELOCITY = 1.0f;
constexpr int32_t PCM_ATTENUATION_DIVISOR = 1;
constexpr int32_t PCM_GAIN_MULTIPLIER = 12;
constexpr int32_t GATE_GAIN_SCALE = 32767;
constexpr size_t GATE_RAMP_SAMPLES = 1024;
constexpr uint32_t AMY_BLOCK_INTERVAL_US =
    (AMY_BLOCK_SIZE * 1000000UL) / AMY_SAMPLE_RATE;
constexpr uint8_t MAX_CATCH_UP_BLOCKS_PER_LOOP = 3;
constexpr size_t STREAM_BUFFER_COUNT = 3;
constexpr size_t AMY_BLOCKS_PER_STREAM_BUFFER = 3;
constexpr size_t STREAM_BUFFER_SAMPLES = AMY_BLOCK_SIZE * AMY_BLOCKS_PER_STREAM_BUFFER;
constexpr uint8_t AUDIO_CHANNEL = 0;

uint32_t lastStatusLogAtMs = 0;
uint32_t lastNoteOnAtMs = 0;
uint32_t currentNoteStartedAtMs = 0;
uint32_t lastAmyRenderAtUs = 0;
uint32_t renderedBlockCount = 0;
uint32_t queuedBufferCount = 0;
uint32_t queuedSampleCount = 0;
uint32_t droppedBufferCount = 0;
uint32_t pacingBlockedCount = 0;
int16_t recentInputPeakSample = 0;
int16_t recentPeakSample = 0;
int32_t gateGain = 0;
int32_t gateTargetGain = 0;
int32_t gateStep = 0;
size_t activeStreamBufferIndex = 0;
size_t activeStreamBufferSamples = 0;
bool amyStarted = false;
bool noteActive = false;
bool muted = false;
bool oscillatorStarted = false;
float activeFrequencyHz = 0.0f;

int16_t streamBuffers[STREAM_BUFFER_COUNT][STREAM_BUFFER_SAMPLES];

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
  M5.Display.println("Mode: AMY tri-buffer");
  M5.Display.printf("AMY: %s\n", amyStarted ? "started" : "not started");
  M5.Display.printf("Frequency: %.2f Hz\n", activeFrequencyHz);
  M5.Display.printf("Queued: %lu\n", static_cast<unsigned long>(queuedBufferCount));
  M5.Display.printf("Dropped: %lu\n", static_cast<unsigned long>(droppedBufferCount));
  M5.Display.printf("Blocked: %lu\n", static_cast<unsigned long>(pacingBlockedCount));
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

void startAmyForPcm() {
  amy_config_t amyConfig = amy_default_config();

  // AMY renders PCM blocks; M5Unified owns the Core Gray speaker output.
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
}

void startSineOscillator(float frequencyHz) {
  amy_event event = amy_default_event();
  event.osc = TEST_OSC_ID;
  event.wave = SINE;
  event.freq_coefs[COEF_CONST] = frequencyHz;
  event.velocity = AMY_TEST_VELOCITY;
  amy_add_event(&event);

  oscillatorStarted = true;
  Serial.printf("amy: sine_started frequency_hz=%.2f\n", frequencyHz);
}

void setGateTarget(int32_t targetGain) {
  gateTargetGain = targetGain;
  const int32_t delta = gateTargetGain - gateGain;
  gateStep = delta / static_cast<int32_t>(GATE_RAMP_SAMPLES);
  if (gateStep == 0 && delta != 0) {
    gateStep = delta > 0 ? 1 : -1;
  }
}

void toneOn(float frequencyHz) {
  if (muted) {
    return;
  }

  if (!oscillatorStarted) {
    startSineOscillator(frequencyHz);
  }

  setGateTarget(GATE_GAIN_SCALE);
  activeFrequencyHz = frequencyHz;
  noteActive = true;
  currentNoteStartedAtMs = millis();
  Serial.printf("gate: fade_in frequency_hz=%.2f\n", frequencyHz);
}

void toneOff() {
  setGateTarget(0);
  activeFrequencyHz = 0.0f;
  noteActive = false;
  Serial.println("gate: fade_out");
}

void toggleMute() {
  muted = !muted;
  if (muted) {
    toneOff();
    M5.Speaker.stop(AUDIO_CHANNEL);
    activeStreamBufferSamples = 0;
    gateGain = 0;
    gateTargetGain = 0;
    gateStep = 0;
  } else {
    lastNoteOnAtMs = millis() - NOTE_ON_INTERVAL_MS;
  }

  Serial.printf("amy: muted=%s\n", muted ? "true" : "false");
  drawScreen(muted ? "muted" : "running");
}

void updateTestNotes() {
  const uint32_t nowMs = millis();

  if (noteActive && nowMs - currentNoteStartedAtMs >= NOTE_DURATION_MS) {
    toneOff();
  }

  if (!noteActive && nowMs - lastNoteOnAtMs >= NOTE_ON_INTERVAL_MS) {
    lastNoteOnAtMs = nowMs;
    toneOn(TEST_FREQUENCY_HZ);
  }
}

bool queueActiveStreamBuffer() {
  if (activeStreamBufferSamples == 0) {
    return true;
  }

  if (M5.Speaker.isPlaying(AUDIO_CHANNEL) >= 2) {
    return false;
  }

  const bool queued = M5.Speaker.playRaw(streamBuffers[activeStreamBufferIndex],
                                         activeStreamBufferSamples,
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
  queuedSampleCount += activeStreamBufferSamples;
  activeStreamBufferIndex = (activeStreamBufferIndex + 1) % STREAM_BUFFER_COUNT;
  activeStreamBufferSamples = 0;
  return true;
}

void appendAmyBlockToStream(int16_t* samples) {
  if (samples == nullptr || muted) {
    return;
  }

  if (activeStreamBufferSamples + AMY_BLOCK_SIZE > STREAM_BUFFER_SAMPLES &&
      !queueActiveStreamBuffer()) {
    return;
  }

  int16_t* output = streamBuffers[activeStreamBufferIndex] + activeStreamBufferSamples;
  int16_t inputPeak = 0;
  int16_t peak = 0;
  for (size_t frame = 0; frame < AMY_BLOCK_SIZE; ++frame) {
    const int32_t left = samples[frame * AMY_NCHANS];
    const int32_t right = samples[(frame * AMY_NCHANS) + 1];
    const int32_t inputMono = (left + right) / 2;
    if (gateGain != gateTargetGain) {
      const bool rising = gateStep > 0;
      gateGain += gateStep;
      if ((rising && gateGain > gateTargetGain) ||
          (!rising && gateGain < gateTargetGain)) {
        gateGain = gateTargetGain;
      }
    }

    int32_t mono = (inputMono * PCM_GAIN_MULTIPLIER) / PCM_ATTENUATION_DIVISOR;
    mono = (mono * gateGain) / GATE_GAIN_SCALE;
    if (mono > INT16_MAX) {
      mono = INT16_MAX;
    } else if (mono < INT16_MIN) {
      mono = INT16_MIN;
    }
    output[frame] = static_cast<int16_t>(mono);

    const int32_t absInputSample = inputMono < 0 ? -inputMono : inputMono;
    if (absInputSample > inputPeak) {
      inputPeak = static_cast<int16_t>(absInputSample);
    }

    const int32_t absSample = mono < 0 ? -mono : mono;
    if (absSample > peak) {
      peak = static_cast<int16_t>(absSample);
    }
  }
  recentInputPeakSample = inputPeak;
  recentPeakSample = peak;

  activeStreamBufferSamples += AMY_BLOCK_SIZE;
  renderedBlockCount++;

  if (activeStreamBufferSamples >= STREAM_BUFFER_SAMPLES &&
      !queueActiveStreamBuffer()) {
    droppedBufferCount++;
    activeStreamBufferSamples = 0;
  }
}

bool canAcceptAmyBlock() {
  if (activeStreamBufferSamples + AMY_BLOCK_SIZE < STREAM_BUFFER_SAMPLES) {
    return true;
  }

  // If this AMY block completes the active stream buffer, only render it when
  // M5Unified has room for the full buffer immediately after the copy.
  return M5.Speaker.isPlaying(AUDIO_CHANNEL) < 2;
}
}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);

  auto config = M5.config();
  M5.begin(config);
  M5.Display.setRotation(1);

  Serial.println();
  Serial.println("AMY Core Gray tri-buffer PCM probe");
  Serial.printf("board_id=%d\n", static_cast<int>(M5.getBoard()));
  Serial.printf("amy: sample_rate=%u block_size=%u channels=%u\n",
                AMY_SAMPLE_RATE,
                AMY_BLOCK_SIZE,
                AMY_NCHANS);
  Serial.printf("stream: buffers=%u samples_per_buffer=%u attenuation=1/%ld\n",
                static_cast<unsigned>(STREAM_BUFFER_COUNT),
                static_cast<unsigned>(STREAM_BUFFER_SAMPLES),
                static_cast<long>(PCM_ATTENUATION_DIVISOR));
  Serial.printf("stream: pcm_gain=%ldx\n", static_cast<long>(PCM_GAIN_MULTIPLIER));
  Serial.printf("stream: amy_block_interval_us=%lu\n",
                static_cast<unsigned long>(AMY_BLOCK_INTERVAL_US));

  configureSpeaker();
  drawScreen("starting AMY");
  startAmyForPcm();
  lastAmyRenderAtUs = micros();
  drawScreen("AMY started");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    toggleMute();
  }

  updateTestNotes();
  const uint32_t nowUs = micros();
  uint8_t catchUpBlocks = 0;
  while (amyStarted && !muted &&
         nowUs - lastAmyRenderAtUs >= AMY_BLOCK_INTERVAL_US &&
         catchUpBlocks < MAX_CATCH_UP_BLOCKS_PER_LOOP) {
    if (!canAcceptAmyBlock()) {
      pacingBlockedCount++;
      lastAmyRenderAtUs += AMY_BLOCK_INTERVAL_US;
      break;
    }

    appendAmyBlockToStream(amy_update());
    lastAmyRenderAtUs += AMY_BLOCK_INTERVAL_US;
    catchUpBlocks++;
  }

  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs >= STATUS_LOG_INTERVAL_MS) {
    lastStatusLogAtMs = nowMs;
    Serial.printf("status: uptime_ms=%lu amy_started=%s muted=%s rendered=%lu queued_buffers=%lu queued_samples=%lu dropped_buffers=%lu pacing_blocked=%lu speaker_queue=%u note_active=%s frequency_hz=%.2f fill_samples=%u gate=%ld input_peak=%d peak_sample=%d\n",
                  static_cast<unsigned long>(nowMs),
                  amyStarted ? "true" : "false",
                  muted ? "true" : "false",
                  static_cast<unsigned long>(renderedBlockCount),
                  static_cast<unsigned long>(queuedBufferCount),
                  static_cast<unsigned long>(queuedSampleCount),
                  static_cast<unsigned long>(droppedBufferCount),
                  static_cast<unsigned long>(pacingBlockedCount),
                  static_cast<unsigned>(M5.Speaker.isPlaying(AUDIO_CHANNEL)),
                  noteActive ? "true" : "false",
                  activeFrequencyHz,
                  static_cast<unsigned>(activeStreamBufferSamples),
                  static_cast<long>(gateGain),
                  static_cast<int>(recentInputPeakSample),
                  static_cast<int>(recentPeakSample));
  }
}
