#include "pet_state.h"

namespace buddy {

namespace {

// 表情→灯/振動的稳态搭配
PetVisual steadyVisual(const PetInputs& in) {
  if (!in.connected) return {Expression::Sleepy, Led::Off, Vibration::None};
  if (in.pendingApprovals > 0) return {Expression::Doubt, Led::RedBlink, Vibration::Pulse};
  if (in.runningSessions > 0) return {Expression::Happy, Led::GreenBreath, Vibration::None};
  return {Expression::Neutral, Led::CyanDim, Vibration::None};
}

// 回落安全:until==0 表示从未触发(0 时刻的瞬态也按未激活处理,符合 setInputs 初值语义)。
bool isActive(uint32_t now_ms, uint32_t until_ms) {
  if (until_ms == 0) return false;
  return static_cast<int32_t>(now_ms - until_ms) < 0;
}

}  // namespace

PetVisual PetStateMachine::update(uint32_t now_ms) const {
  // 瞬态优先级:Shake > Error > Approve > Deny。任一未过期即盖过稳态。
  // 用有符号差值比较,使 millis() 32 位回绕(~49.7 天)时单次回绕仍正确:
  // (int32_t)(now - until) < 0 等价于 "now 在 until 之前"。
  if (isActive(now_ms, shakeUntil_)) return {Expression::Dizzy, Led::RainbowSpin, Vibration::Buzz};
  if (isActive(now_ms, errorUntil_)) return {Expression::Angry, Led::YellowFlash, Vibration::LongBuzz};
  if (isActive(now_ms, approveUntil_)) return {Expression::Love, Led::PinkPulse, Vibration::DoubleTap};
  if (isActive(now_ms, denyUntil_)) return {Expression::Sad, Led::DarkBlue, Vibration::None};
  return steadyVisual(inputs_);
}

}  // namespace buddy
