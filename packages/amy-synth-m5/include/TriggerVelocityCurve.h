#pragma once

#include "StepLevel.h"

struct TriggerVelocityCurve {
  float weak = 0.45f;
  float normal = 0.70f;
  float strong = 1.0f;

  constexpr float velocityFor(StepLevel level) const {
    return level == StepLevel::Weak
               ? weak
               : level == StepLevel::Normal
                     ? normal
                     : level == StepLevel::Strong ? strong : 0.0f;
  }
};

