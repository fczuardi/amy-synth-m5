#pragma once

#include <Arduino.h>

class AmyMonoVoice {
 public:
  void begin(uint8_t synthId, uint8_t voiceCount, uint8_t initialPatch);
  void setPatch(uint8_t patchNumber);
  void noteOn(uint8_t midiNote, float velocity);
  void noteOff();

  uint8_t synthId() const;
  uint8_t voiceCount() const;
  uint8_t patchNumber() const;
  uint8_t activeMidiNote() const;
  bool noteActive() const;

 private:
  uint8_t synthId_ = 0;
  uint8_t voiceCount_ = 1;
  uint8_t patchNumber_ = 0;
  uint8_t activeMidiNote_ = 0;
  bool noteActive_ = false;
  bool begun_ = false;
};
