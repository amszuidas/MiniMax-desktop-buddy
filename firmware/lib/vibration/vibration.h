#pragma once
#include <cstdint>

namespace buddy {

// 振动模式,对应 PetVisual.vibration 的种类。
enum class VibrationPattern { None, Buzz, DoubleTap, Pulse, LongBuzz };

/**
 * 非阻塞振动播放器。play() 载入一个分段脚本 + 起始时刻;update(now) 返回此刻
 * 应输出的强度(0..255),跨段自动推进,结束返回 0。完全不 delay。
 * 纯逻辑,可在 host 上单测;硬件侧每帧 M5.Power.setVibration(player.update(now))。
 */
class VibrationPlayer {
 public:
  void play(VibrationPattern pattern, uint32_t now_ms);
  uint8_t update(uint32_t now_ms) const;

 private:
  VibrationPattern pattern_ = VibrationPattern::None;
  uint32_t startMs_ = 0;
};

}  // namespace buddy
