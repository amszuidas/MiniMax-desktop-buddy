#ifndef UNIT_TEST
#include <M5Unified.h>
#include <Avatar.h>
#include "pet_state.h"
#include "shake_detect.h"
#include "vibration.h"
#include "ble_peripheral.h"
#include "buddy_face.h"
#include "buddy_fx.h"
#include "face_drive.h"
#include <FastLED.h>

using namespace m5avatar;
using namespace buddy_face;

Avatar avatar;
buddy::PetStateMachine sm;
buddy::Expression lastExpression = buddy::Expression::Neutral;
ShakeDetector shake;
buddy::Vibration lastVibration = buddy::Vibration::None;
buddy::VibrationPlayer vibPlayer;
uint8_t lastVibLevel = 255;  // sentinel != any normal level → first write always lands
buddy_ble::BlePeripheral ble;
int lastApprovalId = 0;  // 当前 BLE 状态里的审批 id(0=无),按键用它发事件
m5avatar::Face* buddyFace = nullptr;
bool dizzyEyesOn = false;
uint32_t lastRemindMs = 0;
bool prevHadPending = false;
#define RGB_PIN 32        // Grove Port A 数据脚
#define RGB_COUNT 3       // Unit RGB 板载 3 颗
CRGB leds[RGB_COUNT];

void fillLeds(const CRGB& c) { for (int i = 0; i < RGB_COUNT; i++) leds[i] = c; }

buddy::VibrationPattern toVibPattern(buddy::Vibration v) {
  switch (v) {
    case buddy::Vibration::Buzz:      return buddy::VibrationPattern::Buzz;
    case buddy::Vibration::DoubleTap: return buddy::VibrationPattern::DoubleTap;
    case buddy::Vibration::LongBuzz:  return buddy::VibrationPattern::LongBuzz;
    case buddy::Vibration::Pulse:     return buddy::VibrationPattern::Pulse;
    case buddy::Vibration::None:      return buddy::VibrationPattern::None;
  }
  return buddy::VibrationPattern::None;
}

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

// 底脸库表情:Love/Happy/Dizzy/Sleepy 用 Neutral(符号由 BuddyEffect 自绘,避免库 Effect 双画);
// Sad/Angry/Doubt 用库对应表情(BuddyEffect 对它们 noop,不会双画),保住这三个状态的可读性。
m5avatar::Expression baseLibExpression(buddy::Expression e) {
  switch (e) {
    case buddy::Expression::Sad:   return m5avatar::Expression::Sad;
    case buddy::Expression::Angry: return m5avatar::Expression::Angry;
    case buddy::Expression::Doubt: return m5avatar::Expression::Doubt;
    default:                       return m5avatar::Expression::Neutral; // Neutral/Happy/Dizzy/Love/Sleepy
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  avatar.init();
  buddyFace = makeBuddyFace();
  avatar.setFace(buddyFace);
  // base lib expression is set per-frame in loop() (see baseLibExpression)
  // ⚠️ Core2 默认不给 Grove 5V 供电,必须显式打开,否则 Unit RGB 不亮
  M5.Power.setExtOutput(true);
  FastLED.addLeds<WS2812, RGB_PIN, GRB>(leds, RGB_COUNT);
  FastLED.setBrightness(40);
  ble.begin();
}

void loop() {
  M5.update();
  uint32_t now = millis();

  // IMU 摇晃检测(本地,不经 BLE)
  float ax, ay, az;
  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    if (shake.feed(ax, ay, az, now)) sm.onShake(now);
  }

  // BLE 状态 → 状态机输入。未连接 central 或没收到状态时,显示"未连接"。
  // 关键:central 断连后 latestState() 仍返回旧快照,必须用 isConnected() 覆盖,
  // 否则 Mac 休眠/超距后宠物会一直显示"已连接 + 幽灵审批"。
  buddy_ble::ParsedState ps = ble.latestState();
  if (!ble.isConnected()) ps.inputs.connected = false;
  sm.setInputs(ps.inputs);
  lastApprovalId = (ble.isConnected() && ps.hasApproval) ? ps.approvalId : 0;

  // Relief edge: pending goes from >0 to 0 → trigger "sigh of relief" transient.
  // Only recognize "approvals cleared" edge while still connected — disconnect must not
  // be misinterpreted as "the approval was handled".
  const bool curHadPending = (ble.isConnected() && lastApprovalId > 0);
  if (ble.isConnected() && prevHadPending && !curHadPending) sm.onApprovalsCleared(now);
  prevHadPending = curHadPending;

  // session.error → Angry 脸(error 标志是 Mac 侧瞬态;latch 只在上升沿触发一次)
  static bool errorLatch = false;
  if (ble.isConnected() && ps.hasError) {
    if (!errorLatch) { sm.onSessionError(now); errorLatch = true; }
  } else {
    errorLatch = false;
  }

  // 三键 → 发真实审批事件(仅当有待审批时)。本地立即播放瞬态动画(乐观 UI:
  // 在 Mac 确认前先动画;若 daemon 拒绝,下一次 BLE 状态推送会纠正)。
  // wasHold 先判:长按=永久批准,不被误判为轻按;else-if 防同帧双发。
  if (lastApprovalId > 0) {
    if (M5.BtnB.wasHold())         { ble.sendEvent("always",  lastApprovalId); sm.onApprove(now); }
    else if (M5.BtnB.wasClicked()) { ble.sendEvent("approve", lastApprovalId); sm.onApprove(now); }
    if (M5.BtnA.wasClicked())      { ble.sendEvent("deny",    lastApprovalId); sm.onDeny(now); }
  }

  buddy::PetVisual v = sm.update(now);
  // Feed self-drawn effect shared state (avatar render thread reads g_buddyFx for symbols).
  g_buddyFx.expr = v.expression;
  g_buddyFx.intensity = v.intensity;
  g_buddyFx.nowMs = now;
  // Dizzy enter/exit: swap to spiral eyes (only on change).
  const bool wantDizzy = (v.expression == buddy::Expression::Dizzy);
  if (wantDizzy != dizzyEyesOn) { setDizzyEyes(buddyFace, wantDizzy); dizzyEyesOn = wantDizzy; }
  // Speech bubble text still updated (text fallback to distinguish states).
  if (v.expression != lastExpression) {
    avatar.setExpression(baseLibExpression(v.expression));
    avatar.setSpeechText(buddy::expressionLabel(v.expression));
    lastExpression = v.expression;
  }
  // 非阻塞振动:种类变化时载入脚本;每帧按 now 输出强度(零 delay)。
  if (v.vibration != lastVibration) {
    vibPlayer.play(toVibPattern(v.vibration), now);
    lastVibration = v.vibration;
  }
  // Periodic approval reminder: when connected + pending, re-buzz every kRemindIntervalMs.
  constexpr uint32_t kRemindIntervalMs = 6000;
  if (ble.isConnected() && lastApprovalId > 0) {
    if (lastRemindMs == 0) lastRemindMs = now;
    else if (static_cast<int32_t>(now - (lastRemindMs + kRemindIntervalMs)) >= 0) {
      vibPlayer.play(buddy::VibrationPattern::Pulse, now);
      lastRemindMs = now;
    }
  } else {
    lastRemindMs = 0;
  }
  uint8_t vibLevel = vibPlayer.update(now);
  if (vibLevel != lastVibLevel) { M5.Power.setVibration(vibLevel); lastVibLevel = vibLevel; }
  renderLed(v.led, now);
  delay(16);
}
#endif
