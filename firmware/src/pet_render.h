#pragma once
#ifndef UNIT_TEST
#include <Avatar.h>
#include "pet_state.h"

// 把 buddy::Expression 映射到 m5stack-avatar 的 Expression 并应用到脸上。
// 库内置: Neutral / Happy / Sleepy / Doubt / Sad / Angry。
// 我们暂时用最接近的内置表情近似 Dizzy/Love(M4 再做自绘叠加特效)。
inline void applyExpression(m5avatar::Avatar& avatar, buddy::Expression e) {
  using AE = m5avatar::Expression;
  AE mapped = AE::Neutral;
  switch (e) {
    case buddy::Expression::Sleepy:  mapped = AE::Sleepy;  break;
    case buddy::Expression::Neutral: mapped = AE::Neutral; break;
    case buddy::Expression::Happy:   mapped = AE::Happy;   break;
    case buddy::Expression::Doubt:   mapped = AE::Doubt;   break;
    case buddy::Expression::Dizzy:   mapped = AE::Doubt;   break;  // 临时近似(与 Angry 区分);M4 做螺旋眼
    case buddy::Expression::Love:    mapped = AE::Happy;   break;  // 临时近似;M4 做爱心
    case buddy::Expression::Sad:     mapped = AE::Sad;     break;
    case buddy::Expression::Angry:   mapped = AE::Angry;   break;
  }
  avatar.setExpression(mapped);
}
#endif
