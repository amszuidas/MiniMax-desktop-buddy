#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Avatar.h>
#include "pet_state.h"

// 逐表情把 avatar 的脸参数推到夸张:闭眼/晃头/张嘴/弹跳。
// 每帧调用。每个表情都设"全套"参数(含正常值),所以切换天然复位,不会卡变形态。
//
// ⚠️ eyeOpenRatio 在本库是二值(见 Eye::draw):>0 画整只睁眼、==0 画闭眼横线,
//    中间值不缩放(0.05/0.3/0.4 都渲染成"完全睁开")。所以"真闭眼"只能用 0.0(Sleepy)。
//    眼形变化(Sad 吊眼 / Angry 怒眼)靠底脸库 Expression 路由(main.cpp baseLibExpression),
//    不靠 ratio。Dizzy 用螺旋眼(setDizzyEyes),eyeOpen 对它无效。
// 例外:Neutral 不设 eyeOpen,交还库 autoBlink 接管眨眼(main.cpp 在 Neutral 时开 autoBlink);也不设 breath,交还库 facialLoop 自然呼吸。
// now 用于动态相位(晃头/抖动/弹跳)。
inline void driveFace(m5avatar::Avatar& avatar, buddy::Expression e,
                      uint8_t /*intensity*/, uint32_t now) {
  switch (e) {
    case buddy::Expression::Love: {
      avatar.setRotation(0.0f);
      float bounce = 1.0f + 0.08f * fabsf(sinf(now / 120.0f));
      avatar.setScale(bounce);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.5f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Happy: {
      avatar.setRotation(0.03f * sinf(now / 80.0f));  // slight fast jitter (busy)
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.2f);
      avatar.setBreath(fabsf(sinf(now / 200.0f)));
      break;
    }
    case buddy::Expression::Dizzy: {
      avatar.setScale(1.0f);
      avatar.setRotation(0.15f * sinf(now / 150.0f));
      avatar.setEyeOpenRatio(1.0f);  // spiral eyes ignore ratio; keep open (no-op)
      avatar.setMouthOpenRatio(0.3f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Sleepy: {
      avatar.setRotation(0.0f);
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(0.0f);  // 0 == closed (eyelid line); only 0 closes
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(fabsf(sinf(now / 1400.0f)));
      break;
    }
    case buddy::Expression::Doubt: {
      avatar.setRotation(0.08f);  // fixed head-tilt (curious/questioning)
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Sad: {
      avatar.setRotation(0.05f * sinf(now / 380.0f));  // slow head-shake (no)
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);  // >0 so lib draws Sad droopy-eye shape (via base Expression)
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
      avatar.setMouthOpenRatio(0.0f);
      break;
    }
  }
}
#endif
