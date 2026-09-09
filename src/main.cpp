#include <Arduino.h>
#include <AMY-Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t STATUS_LOG_INTERVAL_MS = 1000;
constexpr uint32_t NOTE_ON_INTERVAL_MS = 2500;
constexpr uint32_t NOTE_DURATION_MS = 900;
constexpr uint8_t AMY_SYNTH_ID = 1;
constexpr uint8_t AMY_PATCH_JUNO = 1;
constexpr uint8_t AMY_POLYPHONY = 4;
constexpr uint8_t TEST_NOTES[] = {48, 52, 55, 60};
constexpr size_t AUDIO_BUFFER_COUNT = 4;
constexpr uint8_t AUDIO_CHANNEL = 0;

uint32_t lastStatusLogAtMs = 0;
uint32_t lastNoteOnAtMs = 0;
uint32_t currentNoteStartedAtMs = 0;
uint32_t renderedBlockCount = 0;
uint32_t queuedBlockCount = 0;
uint32_t droppedBlockCount = 0;
size_t nextNoteIndex = 0;
size_t nextAudioBufferIndex = 0;
bool amyStarted = false;
bool noteActive = false;

int16_t audioBuffers[AUDIO_BUFFER_COUNT][AMY_BLOCK_SIZE * AMY_NCHANS];

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
  M5.Display.println("Mode: local PCM sound");
  M5.Display.printf("AMY: %u Hz %u frames\n", AMY_SAMPLE_RATE, AMY_BLOCK_SIZE);
  M5.Display.print("AMY: ");
  M5.Display.println(amyStarted ? "started" : "not started");
  M5.Display.printf("Rendered: %lu\n", static_cast<unsigned long>(renderedBlockCount));
  M5.Display.printf("Queued: %lu\n", static_cast<unsigned long>(queuedBlockCount));
  M5.Display.printf("Dropped: %lu\n", static_cast<unsigned long>(droppedBlockCount));
  M5.Display.print("Note: ");
  M5.Display.println(noteActive ? "on" : "off");
  M5.Display.print("State: ");
  M5.Display.println(stateLabel);
}

void configureSpeaker() {
  auto speakerConfig = M5.Speaker.config();
  speakerConfig.sample_rate = AMY_SAMPLE_RATE;
  speakerConfig.stereo = true;
  M5.Speaker.config(speakerConfig);
  M5.Speaker.begin();
  M5.Speaker.setVolume(96);
  M5.Speaker.setAllChannelVolume(255);
}

void startAmyForPcm() {
  amy_config_t amyConfig = amy_default_config();

  // AMY renders PCM blocks; M5Unified owns the Core Gray speaker output.
  amyConfig.audio = AMY_AUDIO_IS_NONE;
  amyConfig.features.startup_bleep = 0;
  amyConfig.features.default_synths = 1;
  amyConfig.features.audio_in = 0;
  amyConfig.platform.multithread = 0;
  amyConfig.platform.multicore = 0;

  amy_start(amyConfig);
  amyStarted = true;

  amy_event event = amy_default_event();
  event.synth = AMY_SYNTH_ID;
  event.patch_number = AMY_PATCH_JUNO;
  event.num_voices = AMY_POLYPHONY;
  amy_add_event(&event);
}

void noteOn(uint8_t midiNote) {
  amy_event event = amy_default_event();
  event.synth = AMY_SYNTH_ID;
  event.midi_note = midiNote;
  event.velocity = 0.7f;
  amy_add_event(&event);

  noteActive = true;
  currentNoteStartedAtMs = millis();
  Serial.printf("amy: note_on midi_note=%u\n", midiNote);
}

void noteOff(uint8_t midiNote) {
  amy_event event = amy_default_event();
  event.synth = AMY_SYNTH_ID;
  event.midi_note = midiNote;
  event.velocity = 0.0f;
  amy_add_event(&event);

  noteActive = false;
  Serial.printf("amy: note_off midi_note=%u\n", midiNote);
}

void updateTestNotes() {
  const uint32_t nowMs = millis();

  if (noteActive && nowMs - currentNoteStartedAtMs >= NOTE_DURATION_MS) {
    const size_t activeNoteIndex =
        (nextNoteIndex + (sizeof(TEST_NOTES) / sizeof(TEST_NOTES[0])) - 1) %
        (sizeof(TEST_NOTES) / sizeof(TEST_NOTES[0]));
    noteOff(TEST_NOTES[activeNoteIndex]);
  }

  if (!noteActive && nowMs - lastNoteOnAtMs >= NOTE_ON_INTERVAL_MS) {
    lastNoteOnAtMs = nowMs;
    noteOn(TEST_NOTES[nextNoteIndex]);
    nextNoteIndex = (nextNoteIndex + 1) % (sizeof(TEST_NOTES) / sizeof(TEST_NOTES[0]));
  }
}

void queueAmyAudioBlock(int16_t* samples) {
  if (samples == nullptr) {
    return;
  }

  renderedBlockCount++;

  if (M5.Speaker.isPlaying(AUDIO_CHANNEL) >= 2) {
    droppedBlockCount++;
    return;
  }

  int16_t* outputBuffer = audioBuffers[nextAudioBufferIndex];
  memcpy(outputBuffer,
         samples,
         sizeof(int16_t) * AMY_BLOCK_SIZE * AMY_NCHANS);

  const bool queued = M5.Speaker.playRaw(outputBuffer,
                                         AMY_BLOCK_SIZE * AMY_NCHANS,
                                         AMY_SAMPLE_RATE,
                                         true,
                                         1,
                                         AUDIO_CHANNEL,
                                         false);
  if (queued) {
    queuedBlockCount++;
    nextAudioBufferIndex = (nextAudioBufferIndex + 1) % AUDIO_BUFFER_COUNT;
  } else {
    droppedBlockCount++;
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
  Serial.println("AMY Core Gray local PCM sound probe");
  Serial.printf("board_id=%d\n", static_cast<int>(M5.getBoard()));
  Serial.printf("amy: sample_rate=%u block_size=%u channels=%u\n",
                AMY_SAMPLE_RATE,
                AMY_BLOCK_SIZE,
                AMY_NCHANS);

  configureSpeaker();
  drawScreen("starting AMY");
  startAmyForPcm();

  Serial.println("amy: started with AMY_AUDIO_IS_NONE, routing PCM through M5Unified");
  drawScreen("AMY started");
}

void loop() {
  M5.update();
  updateTestNotes();

  if (amyStarted && M5.Speaker.isPlaying(AUDIO_CHANNEL) < 2) {
    queueAmyAudioBlock(amy_update());
  }

  const uint32_t nowMs = millis();
  if (nowMs - lastStatusLogAtMs >= STATUS_LOG_INTERVAL_MS) {
    lastStatusLogAtMs = nowMs;
    Serial.printf("status: uptime_ms=%lu amy_started=%s rendered=%lu queued=%lu dropped=%lu speaker_queue=%u note_active=%s\n",
                  static_cast<unsigned long>(nowMs),
                  amyStarted ? "true" : "false",
                  static_cast<unsigned long>(renderedBlockCount),
                  static_cast<unsigned long>(queuedBlockCount),
                  static_cast<unsigned long>(droppedBlockCount),
                  static_cast<unsigned>(M5.Speaker.isPlaying(AUDIO_CHANNEL)),
                  noteActive ? "true" : "false");
    drawScreen("running");
  }
}
