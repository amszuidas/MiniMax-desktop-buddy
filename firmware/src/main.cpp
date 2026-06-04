#ifndef UNIT_TEST
#include <M5Unified.h>
#include <Avatar.h>
#include "pet_state.h"
#include "pet_render.h"
#include "shake_detect.h"
#include <FastLED.h>

using namespace m5avatar;

Avatar avatar;
buddy::PetStateMachine sm;
buddy::Expression lastExpression = buddy::Expression::Neutral;
ShakeDetector shake;
buddy::Vibration lastVibration = buddy::Vibration::None;
#define RGB_PIN 32        // Grove Port A 数据脚
#define RGB_COUNT 3       // Unit RGB 板载 3 颗
CRGB leds[RGB_COUNT];

// M1 演示场景索引(中键轮流)
int simScenario = 0;

void pushInputs() {
  sm.setInputs(buddy::demoScenario(simScenario));
}

void fillLeds(const CRGB& c) { for (int i = 0; i < RGB_COUNT; i++) leds[i] = c; }

void renderLed(buddy::Led led, uint32_t now) {
  switch (led) {
    case buddy::Led::Off:         fillLeds(CRGB::Black); break;
    case buddy::Led::CyanDim:     fillLeds(CRGB(0, 40, 40)); break;
    case buddy::Led::GreenBreath: { uint8_t b = (sin8(now / 8)); fillLeds(CRGB(0, b, 0)); break; }
    case buddy::Led::RedBlink:    fillLeds((now / 300) % 2 ? CRGB::Red : CRGB::Black); break;
    case buddy::Led::RainbowSpin: { fill_rainbow(leds, RGB_COUNT, (uint8_t)(now / 5), 80); break; }
    case buddy::Led::PinkPulse:   { uint8_t b = sin8(now / 6); fillLeds(CRGB(b, 0, b / 2)); break; }
    case buddy::Led::DarkBlue:    fillLeds(CRGB(0, 0, 60)); break;
    case buddy::Led::YellowFlash: fillLeds((now / 150) % 2 ? CRGB::Yellow : CRGB::Black); break;
    default:                      fillLeds(CRGB::Black); break;
  }
  FastLED.show();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  avatar.init();
  // ⚠️ Core2 默认不给 Grove 5V 供电,必须显式打开,否则 Unit RGB 不亮
  M5.Power.setExtOutput(true);
  FastLED.addLeds<WS2812, RGB_PIN, GRB>(leds, RGB_COUNT);
  FastLED.setBrightness(40);
  pushInputs();
}

void loop() {
  M5.update();
  uint32_t now = millis();

  // IMU 摇晃检测
  float ax, ay, az;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    if (shake.feed(ax, ay, az, now)) {
      sm.onShake(now);
    }
  }

  // M1 演示交互(BLE 接入前):
  // 中键轮流切换预设场景;左键=批准动画;右键=拒绝动画;摇晃=dizzy(在上方 IMU 段)。
  if (M5.BtnB.wasClicked()) { simScenario = (simScenario + 1) % buddy::kDemoScenarioCount; pushInputs(); }
  if (M5.BtnA.wasClicked()) { sm.onApprove(now); }
  if (M5.BtnC.wasClicked()) { sm.onDeny(now); }

  buddy::PetVisual v = sm.update(now);
  if (v.expression != lastExpression) {
    applyExpression(avatar, v.expression);
    avatar.setSpeechText(buddy::expressionLabel(v.expression));
    lastExpression = v.expression;
  }
  // TODO(M2): 这些 delay() 会阻塞主循环最长 ~400ms(LongBuzz),期间丢按键/IMU。
  // 真板阶段改为非阻塞(millis 状态机或 FreeRTOS timer)。M1 无真板可接受。
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
  renderLed(v.led, now);
  delay(16);
}
#endif
