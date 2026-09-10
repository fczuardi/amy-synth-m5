#include <unity.h>

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

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_clamp_preserves_valid_values);
  RUN_TEST(test_clamp_limits_out_of_range_values);
  RUN_TEST(test_to_octaves_maps_center_to_zero);
  RUN_TEST(test_to_octaves_matches_amy_midi_mapping);
  RUN_TEST(test_to_octaves_clamps_before_mapping);
  return UNITY_END();
}
