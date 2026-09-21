#include <Arduino.h>
#include <M5Unified.h>

#include "AmyAudioActivityGate.h"
#include "AmyM5SpeakerBridge.h"
#include "AmySynthSlot.h"

namespace {
constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint8_t AMY_SYNTH_ID = 1;
constexpr uint8_t AMY_DRUM_KIT_VOICES = 1;
constexpr uint16_t AMY_LEGACY_GM_DRUM_PATCH = 258;
constexpr float DRUM_VELOCITY = 1.0f;
constexpr uint32_t DRUM_TAIL_MS = 1500;
constexpr uint32_t LAYER_COMPARISON_INTERVAL_MS = 700;

enum class BrowserMode {
  Single,
  Layers,
};

struct DrumNote {
  uint8_t midiNote;
  const char* name;
};

constexpr DrumNote DRUM_NOTES[] = {
    {35, "Acoustic Bass Drum"}, {36, "Bass Drum 1"},
    {37, "Side Stick"},         {38, "Acoustic Snare"},
    {39, "Hand Clap"},          {40, "Electric Snare"},
    {41, "Low Floor Tom"},      {42, "Closed Hi-Hat"},
    {43, "High Floor Tom"},     {44, "Pedal Hi-Hat"},
    {45, "Low Tom"},            {46, "Open Hi-Hat"},
    {47, "Low-Mid Tom"},        {48, "Hi-Mid Tom"},
    {49, "Crash Cymbal 1"},     {50, "High Tom"},
    {51, "Ride Cymbal 1"},      {52, "Chinese Cymbal"},
    {53, "Ride Bell"},          {54, "Tambourine"},
    {55, "Splash Cymbal"},      {56, "Cowbell"},
    {57, "Crash Cymbal 2"},     {58, "Vibraslap"},
    {59, "Ride Cymbal 2"},      {60, "Hi Bongo"},
    {61, "Low Bongo"},          {62, "Mute Hi Conga"},
    {63, "Open Hi Conga"},      {64, "Low Conga"},
    {65, "High Timbale"},       {66, "Low Timbale"},
    {67, "High Agogo"},         {68, "Low Agogo"},
    {69, "Cabasa"},             {70, "Maracas"},
    {71, "Short Whistle"},      {72, "Long Whistle"},
    {73, "Short Guiro"},        {74, "Long Guiro"},
    {75, "Claves"},             {76, "Hi Wood Block"},
    {77, "Low Wood Block"},     {78, "Mute Cuica"},
    {79, "Open Cuica"},         {80, "Mute Triangle"},
    {81, "Open Triangle"},
};

constexpr size_t DRUM_NOTE_COUNT = sizeof(DRUM_NOTES) / sizeof(DRUM_NOTES[0]);

constexpr DrumNote LAYER_DRUMS[] = {
    {38, "Acoustic Snare"},
    {42, "Closed Hi-Hat"},
    {55, "Splash Cymbal"},
    {56, "Cowbell"},
    {59, "Ride Cymbal 2"},
    {67, "High Agogo"},
    {68, "Low Agogo"},
    {76, "Hi Wood Block"},
};

constexpr size_t LAYER_DRUM_COUNT =
    sizeof(LAYER_DRUMS) / sizeof(LAYER_DRUMS[0]);

AmyM5SpeakerBridge amyBridge;
AmyAudioActivityGate audioGate(amyBridge);
AmySynthSlot drumSlot;

size_t selectedNoteIndex = 1;
size_t activeLayerCount = 1;
BrowserMode browserMode = BrowserMode::Single;
bool layerComparisonActive = false;
size_t nextComparisonLayerCount = 0;
uint32_t nextComparisonAtMs = 0;
uint32_t triggerCount = 0;
uint32_t pendingTriggerCount = 0;
uint8_t pendingMidiNote = 0;

const DrumNote& selectedDrum() {
  return DRUM_NOTES[selectedNoteIndex];
}

void drawScreen(const char* stateLabel) {
  const DrumNote& drum = selectedDrum();

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("AMY drums");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.printf("Patch: %u\n", AMY_LEGACY_GM_DRUM_PATCH);
  M5.Display.printf("Mode: %s\n",
                    browserMode == BrowserMode::Single ? "single" : "layers");
  if (browserMode == BrowserMode::Single) {
    M5.Display.printf("GM note: %u / %u\n",
                      drum.midiNote,
                      static_cast<unsigned>(DRUM_NOTE_COUNT));
    M5.Display.println(drum.name);
  } else {
    M5.Display.printf("Layers: %u / %u\n",
                      static_cast<unsigned>(activeLayerCount),
                      static_cast<unsigned>(LAYER_DRUM_COUNT));
    M5.Display.printf("Last: %u %s\n",
                      LAYER_DRUMS[activeLayerCount - 1].midiNote,
                      LAYER_DRUMS[activeLayerCount - 1].name);
  }
  M5.Display.println();
  M5.Display.println("A click: trigger");
  M5.Display.println("A hold: change mode");
  M5.Display.println("B/C: down/up");
  M5.Display.println();
  M5.Display.printf("State: %s\n", stateLabel);
}

void selectPreviousDrum() {
  selectedNoteIndex =
      (selectedNoteIndex + DRUM_NOTE_COUNT - 1) % DRUM_NOTE_COUNT;
  drawScreen("ready");
  Serial.printf("drum: selected midi_note=%u name=%s\n",
                selectedDrum().midiNote,
                selectedDrum().name);
}

void selectNextDrum() {
  selectedNoteIndex = (selectedNoteIndex + 1) % DRUM_NOTE_COUNT;
  drawScreen("ready");
  Serial.printf("drum: selected midi_note=%u name=%s\n",
                selectedDrum().midiNote,
                selectedDrum().name);
}

void triggerSelectedDrum() {
  const uint8_t midiNote = selectedDrum().midiNote;
  audioGate.wake(DRUM_TAIL_MS);
  drumSlot.noteOn(midiNote, DRUM_VELOCITY);

  triggerCount++;
  pendingTriggerCount++;
  pendingMidiNote = midiNote;
}

void triggerLayerCount(size_t layerCount) {
  audioGate.wake(DRUM_TAIL_MS);
  for (size_t index = 0; index < layerCount; index++) {
    drumSlot.noteOn(LAYER_DRUMS[index].midiNote, DRUM_VELOCITY);
  }

  triggerCount++;
  pendingTriggerCount++;
  pendingMidiNote = LAYER_DRUMS[layerCount - 1].midiNote;
}

void startLayerComparison() {
  layerComparisonActive = true;
  nextComparisonLayerCount = 1;
  nextComparisonAtMs = millis();
}

void updateLayerComparison() {
  if (!layerComparisonActive ||
      static_cast<int32_t>(millis() - nextComparisonAtMs) < 0) {
    return;
  }

  triggerLayerCount(nextComparisonLayerCount);
  nextComparisonLayerCount++;
  if (nextComparisonLayerCount > activeLayerCount) {
    layerComparisonActive = false;
    return;
  }

  nextComparisonAtMs += LAYER_COMPARISON_INTERVAL_MS;
}

void triggerCurrentMode() {
  if (browserMode == BrowserMode::Single) {
    triggerSelectedDrum();
    return;
  }

  startLayerComparison();
}

void toggleBrowserMode() {
  browserMode = browserMode == BrowserMode::Single
                    ? BrowserMode::Layers
                    : BrowserMode::Single;
  drawScreen("ready");
  Serial.printf("drum: mode=%s layers=%u\n",
                browserMode == BrowserMode::Single ? "single" : "layers",
                static_cast<unsigned>(activeLayerCount));
}

void decreaseSelection() {
  if (browserMode == BrowserMode::Single) {
    selectPreviousDrum();
    return;
  }

  if (activeLayerCount > 1) {
    activeLayerCount--;
  }
  drawScreen("ready");
  Serial.printf("drum: layers=%u\n", static_cast<unsigned>(activeLayerCount));
}

void increaseSelection() {
  if (browserMode == BrowserMode::Single) {
    selectNextDrum();
    return;
  }

  if (activeLayerCount < LAYER_DRUM_COUNT) {
    activeLayerCount++;
  }
  drawScreen("ready");
  Serial.printf("drum: layers=%u\n", static_cast<unsigned>(activeLayerCount));
}

void flushDeferredDiagnostics() {
  if (pendingTriggerCount == 0) {
    return;
  }

  Serial.printf(
      "drum: triggered mode=%s layers=%u total=%lu pending=%lu last_midi_note=%u rendered=%lu queued=%lu dropped=%lu blocked=%lu\n",
      browserMode == BrowserMode::Single ? "single" : "layers",
      browserMode == BrowserMode::Single
          ? 1U
          : static_cast<unsigned>(activeLayerCount),
      static_cast<unsigned long>(triggerCount),
      static_cast<unsigned long>(pendingTriggerCount),
      pendingMidiNote,
      static_cast<unsigned long>(amyBridge.renderedBlockCount()),
      static_cast<unsigned long>(amyBridge.queuedBufferCount()),
      static_cast<unsigned long>(amyBridge.droppedBufferCount()),
      static_cast<unsigned long>(amyBridge.queueBlockedCount()));
  pendingTriggerCount = 0;
}

void updateButtons() {
  if (M5.BtnA.wasClicked()) {
    triggerCurrentMode();
  }
  if (M5.BtnA.wasHold() && !audioGate.awake()) {
    toggleBrowserMode();
  }
  if (M5.BtnB.wasClicked() && !audioGate.awake()) {
    decreaseSelection();
  }
  if (M5.BtnC.wasClicked() && !audioGate.awake()) {
    increaseSelection();
  }
}
}  // namespace

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(300);

  auto config = M5.config();
  M5.begin(config);
  M5.Display.setRotation(1);

  drawScreen("starting AMY");
  amyBridge.begin();
  drumSlot.begin(AMY_SYNTH_ID,
                 AMY_DRUM_KIT_VOICES,
                 AMY_LEGACY_GM_DRUM_PATCH);

  Serial.println();
  Serial.println("AMY Core Gray manual drum browser");
  Serial.printf(
      "amy: sample_rate=%u block_size=%u channels=%u synth=%u patch=%u voices=%u\n",
      AMY_SAMPLE_RATE,
      AMY_BLOCK_SIZE,
      AMY_NCHANS,
      AMY_SYNTH_ID,
      AMY_LEGACY_GM_DRUM_PATCH,
      AMY_DRUM_KIT_VOICES);
  Serial.printf("drum: selected midi_note=%u name=%s\n",
                selectedDrum().midiNote,
                selectedDrum().name);
  drawScreen("ready");
}

void loop() {
  M5.update();
  updateButtons();
  updateLayerComparison();

  const bool wasAwake = audioGate.awake();
  const bool isAwake = audioGate.update(false);
  if (wasAwake && !isAwake) {
    flushDeferredDiagnostics();
    drawScreen("ready");
  }
}
