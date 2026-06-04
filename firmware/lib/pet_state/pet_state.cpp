#include "pet_state.h"

namespace buddy {

namespace {

// 表情→灯/振動的稳态搭配
PetVisual steadyVisual(const PetInputs& in) {
  if (!in.connected) return {Expression::Sleepy, Led::Off, Vibration::None, 0};
  if (in.pendingApprovals > 0) return {Expression::Doubt, Led::RedBlink, Vibration::Pulse, 0};
  if (in.runningSessions > 0) {
    int n = in.runningSessions > 3 ? 3 : in.runningSessions;
    uint8_t intensity = static_cast<uint8_t>(n * 80);
    return {Expression::Happy, Led::GreenBreath, Vibration::None, intensity};
  }
  return {Expression::Neutral, Led::CyanDim, Vibration::None, 0};
}

// 回落安全:until==0 表示从未触发(0 时刻的瞬态也按未激活处理,符合 setInputs 初值语义)。
bool isActive(uint32_t now_ms, uint32_t until_ms) {
  if (until_ms == 0) return false;
  return static_cast<int32_t>(now_ms - until_ms) < 0;
}

}  // namespace

PetVisual PetStateMachine::update(uint32_t now_ms) const {
  // 瞬态优先级:Shake > Error > Approve > Deny > Relief。任一未过期即盖过稳态。
  // 用有符号差值比较,使 millis() 32 位回绕(~49.7 天)时单次回绕仍正确:
  // (int32_t)(now - until) < 0 等价于 "now 在 until 之前"。
  if (isActive(now_ms, shakeUntil_)) return {Expression::Dizzy, Led::RainbowSpin, Vibration::Buzz, 0};
  if (isActive(now_ms, errorUntil_)) return {Expression::Angry, Led::YellowFlash, Vibration::LongBuzz, 0};
  if (isActive(now_ms, approveUntil_)) return {Expression::Love, Led::PinkPulse, Vibration::DoubleTap, 0};
  if (isActive(now_ms, denyUntil_)) return {Expression::Sad, Led::DarkBlue, Vibration::None, 0};
  if (isActive(now_ms, reliefUntil_)) return {Expression::Happy, Led::GreenBreath, Vibration::None, 0};

  PetVisual sv = steadyVisual(inputs_);
  const bool isIdle = (sv.expression == Expression::Neutral);
  if (isIdle) {
    if (!idleTracked_) { idleTracked_ = true; idleSinceMs_ = now_ms; }
    else if (static_cast<int32_t>(now_ms - (idleSinceMs_ + kDrowsyAfterMs)) >= 0) {
      return {Expression::Sleepy, Led::Off, Vibration::None, 0};
    }
  } else {
    idleTracked_ = false;
  }
  return sv;
}

const char* expressionLabel(Expression e) {
  switch (e) {
    case Expression::Sleepy:  return "zzz";
    case Expression::Neutral: return "idle";
    case Expression::Happy:   return "busy";
    case Expression::Doubt:   return "approve?";
    case Expression::Dizzy:   return "dizzy";
    case Expression::Love:    return "approved";
    case Expression::Sad:     return "denied";
    case Expression::Angry:   return "error";
  }
  return "";
}

PetInputs demoScenario(int index) {
  switch (((index % kDemoScenarioCount) + kDemoScenarioCount) % kDemoScenarioCount) {
    case 0:  return {.connected = true,  .runningSessions = 0, .pendingApprovals = 0}; // idle
    case 1:  return {.connected = true,  .runningSessions = 1, .pendingApprovals = 0}; // busy
    case 2:  return {.connected = true,  .runningSessions = 0, .pendingApprovals = 1}; // approve?
    default: return {.connected = false, .runningSessions = 0, .pendingApprovals = 0}; // disconnected
  }
}

}  // namespace buddy
