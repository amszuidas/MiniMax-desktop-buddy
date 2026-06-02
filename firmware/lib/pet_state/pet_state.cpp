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

}  // namespace

PetVisual PetStateMachine::update(uint32_t now_ms) const {
  // 瞬态优先级:Shake > Error > Approve > Deny。任一未过期即盖过稳态。
  if (now_ms < shakeUntil_) return {Expression::Dizzy, Led::RainbowSpin, Vibration::Buzz};
  if (now_ms < errorUntil_) return {Expression::Angry, Led::YellowFlash, Vibration::LongBuzz};
  if (now_ms < approveUntil_) return {Expression::Love, Led::PinkPulse, Vibration::DoubleTap};
  if (now_ms < denyUntil_) return {Expression::Sad, Led::DarkBlue, Vibration::None};
  return steadyVisual(inputs_);
}

}  // namespace buddy
