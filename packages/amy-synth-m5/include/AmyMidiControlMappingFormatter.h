#pragma once

#include <cstddef>
#include <cstdio>

#include "AmyMidiControlMapping.h"

inline char amyModulationTargetParameter(AmyModulationTarget target) {
  switch (target) {
    case AmyModulationTarget::Frequency: return 'f';
    case AmyModulationTarget::Amplitude: return 'a';
    case AmyModulationTarget::FilterFrequency: return 'F';
    case AmyModulationTarget::Duty: return 'd';
    case AmyModulationTarget::Pan: return 'Q';
  }
  return '\0';
}

inline bool formatAmyMidiControlMessage(
    uint8_t synthId,
    const AmyMidiControlMapping& mapping,
    char* buffer,
    size_t bufferSize,
    size_t& messageLength) {
  if (mapping.midiChannel > 15 || mapping.controller > 127 ||
      buffer == nullptr || bufferSize == 0) {
    return false;
  }

  const char parameter = amyModulationTargetParameter(mapping.target);
  if (parameter == '\0') return false;

  const char* sourceCoefficientPrefix =
      mapping.source == AmyModulationSource::Mod1 ? ",,,,,,,,," : ",,,,,";

  const int length = std::snprintf(
      buffer, bufferSize, "i%uv%u%c%s%sZ",
      static_cast<unsigned>(synthId),
      static_cast<unsigned>(mapping.targetOscillator),
      parameter, sourceCoefficientPrefix, "%v");
  if (length <= 0 || static_cast<size_t>(length) >= bufferSize) return false;

  messageLength = static_cast<size_t>(length);
  return true;
}
