#pragma once

#include <cstdint>

enum class AmyModulationTarget {
  Frequency,
  Amplitude,
  FilterFrequency,
  Duty,
  Pan,
};

// Describes one MIDI CC mapping for the configured AMY channel and patch. The
// oscillator index is relative to the synth slot, and the patch must provide
// a compatible mod_source route for the selected target.
struct AmyMidiControlMapping {
  uint8_t midiChannel = 0;
  uint8_t controller = 0;
  uint8_t targetOscillator = 0;
  AmyModulationTarget target = AmyModulationTarget::Frequency;
  float coefficientAtMinimum = 0.0f;
  float coefficientAtMaximum = 0.0f;
};
