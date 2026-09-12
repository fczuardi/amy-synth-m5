#include <unity.h>

#include <cstring>

#include "AmyMidiControlMappingFormatter.h"
#include "AmyPitchBend.h"

void test_clamp_preserves_valid_values() {
  TEST_ASSERT_EQUAL_INT16(0, AmyPitchBend::clamp(0));
  TEST_ASSERT_EQUAL_INT16(8191, AmyPitchBend::clamp(8191));
  TEST_ASSERT_EQUAL_INT16(-8192, AmyPitchBend::clamp(-8192));
}

void test_clamp_limits_out_of_range_values() {
  TEST_ASSERT_EQUAL_INT16(8191, AmyPitchBend::clamp(9000));
  TEST_ASSERT_EQUAL_INT16(-8192, AmyPitchBend::clamp(-9000));
}

void test_to_octaves_maps_center_to_zero() {
  TEST_ASSERT_FLOAT_WITHIN(0.000001f, 0.0f, AmyPitchBend::toOctaves(0));
}

void test_to_octaves_matches_amy_midi_mapping() {
  TEST_ASSERT_FLOAT_WITHIN(
      0.000001f,
      8191.0f / (6.0f * 8192.0f),
      AmyPitchBend::toOctaves(8191));
  TEST_ASSERT_FLOAT_WITHIN(
      0.000001f,
      -8192.0f / (6.0f * 8192.0f),
      AmyPitchBend::toOctaves(-8192));
}

void test_to_octaves_clamps_before_mapping() {
  TEST_ASSERT_FLOAT_WITHIN(
      0.000001f,
      AmyPitchBend::toOctaves(8191),
      AmyPitchBend::toOctaves(9000));
  TEST_ASSERT_FLOAT_WITHIN(
      0.000001f,
      AmyPitchBend::toOctaves(-8192),
      AmyPitchBend::toOctaves(-9000));
}

void test_mapping_formatter_encodes_all_targets() {
  const AmyModulationTarget targets[] = {
      AmyModulationTarget::Frequency,
      AmyModulationTarget::Amplitude,
      AmyModulationTarget::FilterFrequency,
      AmyModulationTarget::Duty,
      AmyModulationTarget::Pan,
  };
  const char parameters[] = {'f', 'a', 'F', 'd', 'Q'};

  for (size_t i = 0; i < 5; ++i) {
    AmyMidiControlMapping mapping{
        .midiChannel = 1,
        .controller = 1,
        .targetOscillator = 3,
        .target = targets[i],
    };
    char message[64] = {};
    size_t length = 0;

    TEST_ASSERT_TRUE(formatAmyMidiControlMessage(
        1, mapping, message, sizeof(message), length));
    char expected[32] = {};
    std::snprintf(expected, sizeof(expected), "i1v3%c,,,,,%%vZ", parameters[i]);
    TEST_ASSERT_EQUAL_STRING(expected, message);
    TEST_ASSERT_EQUAL_UINT(std::strlen(expected), length);
  }
}

void test_mapping_formatter_targets_mod1_coefficient() {
  AmyMidiControlMapping mapping{
      .midiChannel = 1,
      .controller = 1,
      .targetOscillator = 2,
      .source = AmyModulationSource::Mod1,
      .target = AmyModulationTarget::Frequency,
  };
  char message[64] = {};
  size_t length = 0;

  TEST_ASSERT_TRUE(formatAmyMidiControlMessage(
      1, mapping, message, sizeof(message), length));
  TEST_ASSERT_EQUAL_STRING("i1v2f,,,,,,,,,%vZ", message);
  TEST_ASSERT_EQUAL_UINT(std::strlen("i1v2f,,,,,,,,,%vZ"), length);
}

void test_mapping_formatter_rejects_invalid_channel() {
  AmyMidiControlMapping mapping{.midiChannel = 16, .controller = 1};
  char message[64] = {};
  size_t length = 0;

  TEST_ASSERT_FALSE(formatAmyMidiControlMessage(
      1, mapping, message, sizeof(message), length));
}

void test_mapping_formatter_rejects_invalid_controller() {
  AmyMidiControlMapping mapping{.midiChannel = 0, .controller = 128};
  char message[64] = {};
  size_t length = 0;

  TEST_ASSERT_FALSE(formatAmyMidiControlMessage(
      1, mapping, message, sizeof(message), length));
}

void test_mapping_formatter_rejects_small_buffer() {
  AmyMidiControlMapping mapping{.targetOscillator = 3};
  char message[4] = {};
  size_t length = 0;

  TEST_ASSERT_FALSE(formatAmyMidiControlMessage(
      1, mapping, message, sizeof(message), length));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_clamp_preserves_valid_values);
  RUN_TEST(test_clamp_limits_out_of_range_values);
  RUN_TEST(test_to_octaves_maps_center_to_zero);
  RUN_TEST(test_to_octaves_matches_amy_midi_mapping);
  RUN_TEST(test_to_octaves_clamps_before_mapping);
  RUN_TEST(test_mapping_formatter_encodes_all_targets);
  RUN_TEST(test_mapping_formatter_targets_mod1_coefficient);
  RUN_TEST(test_mapping_formatter_rejects_invalid_channel);
  RUN_TEST(test_mapping_formatter_rejects_invalid_controller);
  RUN_TEST(test_mapping_formatter_rejects_small_buffer);
  return UNITY_END();
}
