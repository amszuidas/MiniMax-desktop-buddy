#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Avatar.h>
#include "pet_state.h"

// 逐表情把 avatar 的脸参数推到夸张:睁眼/闭眼/张嘴/晃头/弹跳。
// 每帧调用。每个表情都设"全套"参数(含正常值),所以切换天然复位,不会卡变形态。
// 例外:Neutral 设 eyeOpen=1.0 睁眼(autoBlink 已关,须显式睁),但不设 breath,交还库 facialLoop 自然呼吸。
// now 用于动态相位(晃头/抖动/弹跳)。
inline void driveFace(m5avatar::Avatar& avatar, buddy::Expression e,
                      uint8_t /*intensity*/, uint32_t now) {
  switch (e) {
    case buddy::Expression::Love: {
      avatar.setRotation(0.0f);
      float bounce = 1.0f + 0.08f * fabsf(sinf(now / 120.0f));
      avatar.setScale(bounce);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.8f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Happy: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.2f);
      avatar.setBreath(fabsf(sinf(now / 200.0f)));
      break;
    }
    case buddy::Expression::Dizzy: {
      avatar.setScale(1.0f);
      avatar.setRotation(0.15f * sinf(now / 150.0f));
      avatar.setEyeOpenRatio(0.4f);
      avatar.setMouthOpenRatio(0.3f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Sleepy: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(0.05f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(fabsf(sinf(now / 1400.0f)));
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
      avatar.setEyeOpenRatio(0.3f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Angry: {
      avatar.setScale(1.0f);
      avatar.setRotation(0.05f * sinf(now / 40.0f));
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.4f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Neutral:
    default: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);  // autoBlink is off, so open eyes explicitly
      avatar.setMouthOpenRatio(0.0f);
      break;
    }
  }
}
#endif
