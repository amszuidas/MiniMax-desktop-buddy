#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Avatar.h>
#include "pet_state.h"

// The approved concept art is a full-screen composition. Keep the avatar-level
// transform stable so rotation/scale never crops edge decorations; motion lives
// inside the custom drawables instead.
inline void driveFace(m5avatar::Avatar& avatar, buddy::Expression e,
                      uint8_t /*intensity*/, uint32_t now) {
  (void)now;
  switch (e) {
    case buddy::Expression::Love: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.62f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Happy: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.24f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Dizzy: {
      avatar.setScale(1.0f);
      avatar.setRotation(0.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.38f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Sleepy: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(0.0f);  // 0 == closed (eyelid line); only 0 closes
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Doubt: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Sad: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Angry: {
      avatar.setScale(1.0f);
      avatar.setRotation(0.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.48f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Neutral:
    default: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setMouthOpenRatio(0.0f);
      break;
    }
  }
}
#endif
