#ifndef UNIT_TEST
#include <M5Unified.h>
#include <Avatar.h>
#include "pet_state.h"
#include "pet_render.h"

using namespace m5avatar;

Avatar avatar;
buddy::PetStateMachine sm;
buddy::Expression lastExpression = buddy::Expression::Neutral;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  avatar.init();
  // 初始:已连接空闲(M1 先给个非未连接的稳态,方便看脸;Task 5 用按键改)
  sm.setInputs({.connected = true, .runningSessions = 0, .pendingApprovals = 0});
}

void loop() {
  M5.update();
  buddy::PetVisual v = sm.update(millis());
  if (v.expression != lastExpression) {
    applyExpression(avatar, v.expression);
    lastExpression = v.expression;
  }
  delay(16);
}
#endif
