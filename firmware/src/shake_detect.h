#pragma once
#include <cmath>
#include <cstdint>

// 简单摇晃检测:加速度模长偏离 1g 超过阈值,且距上次触发已过冷却期。
// 纯逻辑(不依赖硬件),便于将来 host 测;M1 仅随固件编译。
class ShakeDetector {
 public:
  // ax/ay/az 单位 g;返回 true 表示这一刻判定为"摇晃"。
  bool feed(float ax, float ay, float az, uint32_t now_ms) {
    float mag = std::sqrt(ax * ax + ay * ay + az * az);
    float deviation = std::fabs(mag - 1.0f);
    if (deviation > kThreshold && (now_ms - lastTrigger_) > kCooldownMs) {
      lastTrigger_ = now_ms;
      return true;
    }
    return false;
  }

 private:
  static constexpr float kThreshold = 1.2f;   // g;越大越"用力才触发"
  static constexpr uint32_t kCooldownMs = 1500;
  uint32_t lastTrigger_ = 0;
};
