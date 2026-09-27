#include <unity.h>

#include "TriggerVelocityCurve.h"

void test_default_curve_preserves_authored_level_order() {
  constexpr TriggerVelocityCurve curve;
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, curve.velocityFor(StepLevel::Off));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.45f, curve.velocityFor(StepLevel::Weak));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.70f, curve.velocityFor(StepLevel::Normal));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, curve.velocityFor(StepLevel::Strong));
}

void test_voice_curve_can_attenuate_a_dominant_sound() {
  constexpr TriggerVelocityCurve curve{0.10f, 0.20f, 0.35f};
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.10f, curve.velocityFor(StepLevel::Weak));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.20f, curve.velocityFor(StepLevel::Normal));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.35f, curve.velocityFor(StepLevel::Strong));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_default_curve_preserves_authored_level_order);
  RUN_TEST(test_voice_curve_can_attenuate_a_dominant_sound);
  return UNITY_END();
}
