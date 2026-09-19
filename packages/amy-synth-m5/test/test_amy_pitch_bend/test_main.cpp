#include <unity.h>

#include <cstring>

#include "AmyMidiControlMappingFormatter.h"
#include "AmyM5MonophonicSynthConfiguration.h"
#include "AmyM5PitchBendPolicy.h"
#include "AmyPitchBend.h"

void test_pitch_bend_policy_rejects_event_while_idle() {
  TEST_ASSERT_FALSE(amyM5ShouldApplyPitchBend(false, 2, 2));
}

void test_pitch_bend_policy_accepts_active_channel() {
  TEST_ASSERT_TRUE(amyM5ShouldApplyPitchBend(true, 2, 2));
}

void test_pitch_bend_policy_rejects_other_channel() {
  TEST_ASSERT_FALSE(amyM5ShouldApplyPitchBend(true, 2, 11));
}

void test_configuration_maps_first_channel() {
  AmyM5MonophonicSynthConfiguration configuration{};
  configuration.patches[0] = 19;
  uint16_t patch = 0;

  TEST_ASSERT_TRUE(amyM5PatchForMidiChannel(configuration, 0, patch));
  TEST_ASSERT_EQUAL_UINT16(19, patch);
}

void test_configuration_maps_last_channel() {
  AmyM5MonophonicSynthConfiguration configuration{};
  configuration.patches[15] = 127;
  uint16_t patch = 0;

  TEST_ASSERT_TRUE(amyM5PatchForMidiChannel(configuration, 15, patch));
  TEST_ASSERT_EQUAL_UINT16(127, patch);
}

void test_configuration_maps_middle_channels() {
  AmyM5MonophonicSynthConfiguration configuration{};
  configuration.patches[3] = 300;
  configuration.patches[8] = 1024;
  uint16_t patch = 0;

  TEST_ASSERT_TRUE(amyM5PatchForMidiChannel(configuration, 3, patch));
  TEST_ASSERT_EQUAL_UINT16(300, patch);
  TEST_ASSERT_TRUE(amyM5PatchForMidiChannel(configuration, 8, patch));
  TEST_ASSERT_EQUAL_UINT16(1024, patch);
}

void test_configuration_accepts_patch_zero() {
  AmyM5MonophonicSynthConfiguration configuration{};
  uint16_t patch = 99;

  TEST_ASSERT_TRUE(amyM5PatchForMidiChannel(configuration, 4, patch));
  TEST_ASSERT_EQUAL_UINT16(0, patch);
}

void test_configuration_rejects_malformed_channel() {
  AmyM5MonophonicSynthConfiguration configuration{};
  uint16_t patch = 99;

  TEST_ASSERT_FALSE(amyM5PatchForMidiChannel(configuration, 16, patch));
  TEST_ASSERT_EQUAL_UINT16(99, patch);
}

void test_configuration_has_all_midi_channels() {
  TEST_ASSERT_EQUAL_UINT(16, AMY_M5_MIDI_CHANNEL_COUNT);
}

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
  RUN_TEST(test_pitch_bend_policy_rejects_event_while_idle);
  RUN_TEST(test_pitch_bend_policy_accepts_active_channel);
  RUN_TEST(test_pitch_bend_policy_rejects_other_channel);
  RUN_TEST(test_configuration_maps_first_channel);
  RUN_TEST(test_configuration_maps_last_channel);
  RUN_TEST(test_configuration_maps_middle_channels);
  RUN_TEST(test_configuration_accepts_patch_zero);
  RUN_TEST(test_configuration_rejects_malformed_channel);
  RUN_TEST(test_configuration_has_all_midi_channels);
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
