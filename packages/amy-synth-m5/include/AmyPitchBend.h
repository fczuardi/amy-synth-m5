#pragma once

#include <cstdint>

// Pure helpers for AMY's MIDI pitch-bend mapping.
//
// AMY represents pitch bend in octave units. Its MIDI handler maps the
// centered MIDI bend range to roughly +/-2 semitones by dividing by
// 6 * 8192.
class AmyPitchBend {
 public:
  static constexpr int16_t MIN_VALUE = -8192;
  static constexpr int16_t MAX_VALUE = 8191;

  static int16_t clamp(int16_t value) {
    if (value < MIN_VALUE) {
      return MIN_VALUE;
    }
    if (value > MAX_VALUE) {
      return MAX_VALUE;
    }
    return value;
  }

  static float toOctaves(int16_t value) {
    return static_cast<float>(clamp(value)) / (6.0f * 8192.0f);
  }
};
