#ifndef UNIT_TEST
#include <M5Unified.h>
#include <Avatar.h>
#include "pet_state.h"
#include "pet_render.h"
#include "shake_detect.h"

using namespace m5avatar;

Avatar avatar;
buddy::PetStateMachine sm;
buddy::Expression lastExpression = buddy::Expression::Neutral;
ShakeDetector shake;
buddy::Vibration lastVibration = buddy::Vibration::None;

// M1 模拟状态(M2 由 BLE 取代)
bool simConnected = true;
int simRunning = 0;
int simPending = 0;

void pushInputs() {
  sm.setInputs({.connected = simConnected,
                .runningSessions = simRunning,
                .pendingApprovals = simPending});
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  avatar.init();
  pushInputs();
}

void loop() {
  M5.update();

  // IMU 摇晃检测
  float ax, ay, az;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    if (shake.feed(ax, ay, az, millis())) {
      sm.onShake(millis());
    }
  }

  // BtnA 轻按:切换连接;BtnB 轻按:+1 待审批,长按:清空待审批;
  // BtnC 轻按:切换运行会话。
  if (M5.BtnA.wasClicked()) { simConnected = !simConnected; pushInputs(); }
  if (M5.BtnB.wasClicked()) { simPending += 1; pushInputs(); }
  if (M5.BtnB.wasHold())    { simPending = 0; sm.onApprove(millis()); pushInputs(); }
  if (M5.BtnC.wasClicked()) { simRunning = simRunning > 0 ? 0 : 1; pushInputs(); }

  buddy::PetVisual v = sm.update(millis());
  if (v.expression != lastExpression) {
    applyExpression(avatar, v.expression);
    lastExpression = v.expression;
  }
  // 振动:在种类发生变化的瞬间触发一次对应节奏(避免每帧重复触发)
  if (v.vibration != lastVibration) {
    switch (v.vibration) {
      case buddy::Vibration::Buzz:      M5.Power.setVibration(180); delay(120); M5.Power.setVibration(0); break;
      case buddy::Vibration::DoubleTap: M5.Power.setVibration(160); delay(80);  M5.Power.setVibration(0); delay(80); M5.Power.setVibration(160); delay(80); M5.Power.setVibration(0); break;
      case buddy::Vibration::LongBuzz:  M5.Power.setVibration(220); delay(400); M5.Power.setVibration(0); break;
      case buddy::Vibration::Pulse:     M5.Power.setVibration(140); delay(100); M5.Power.setVibration(0); break;
      case buddy::Vibration::None:      M5.Power.setVibration(0); break;
    }
    lastVibration = v.vibration;
  }
  delay(16);
}
#endif
