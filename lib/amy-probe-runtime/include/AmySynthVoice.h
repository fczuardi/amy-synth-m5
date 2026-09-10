#pragma once

#include <Arduino.h>

// Owns the AMY event state for one synth slot.
//
// This class deliberately does not know about M5Unified, the speaker, PCM
// buffers, buttons, display refresh, or output gain. It only translates a small
// musical interface into AMY patch and note events.
//
// Monophonic or polyphonic policy belongs above this wrapper. This class can
// remember the latest note it started, but noteOn() does not stop any previous
// note on its own.
class AmySynthVoice {
 public:
  // Configures the AMY synth slot and immediately applies the initial patch.
  void begin(uint8_t synthId, uint8_t voiceCount, uint8_t initialPatch);

  // Selects one of AMY's patch numbers for this synth.
  void setPatch(uint8_t patchNumber);

  // Sends a note-on event for this synth slot.
  void noteOn(uint8_t midiNote, float velocity);

  // Sends note-off for the requested MIDI note.
  void noteOff(uint8_t midiNote);

  // Sets AMY's global pitch bend using MIDI's signed bend range.
  //
  // AMY stores pitch bend in octave units. This method follows AMY's MIDI
  // handler mapping: -8192..8191 becomes roughly -2..+2 semitones.
  void setPitchBend(int16_t value);

  // Releases the latest note started through this wrapper, if there is one.
  void stopActiveNote();

  uint8_t synthId() const;
  uint8_t voiceCount() const;
  uint8_t patchNumber() const;
  uint8_t activeMidiNote() const;
  int16_t pitchBend() const;
  bool noteActive() const;

 private:
  static int16_t clampedPitchBend(int16_t value);
  static float pitchBendOctaves(int16_t value);

  uint8_t synthId_ = 0;
  uint8_t voiceCount_ = 1;
  uint8_t patchNumber_ = 0;
  uint8_t activeMidiNote_ = 0;
  int16_t pitchBend_ = 0;
  bool noteActive_ = false;
  bool begun_ = false;
};
