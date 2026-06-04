#pragma once
#include <cstdint>

namespace buddy {

// 宠物表情(底层脸,后续映射到 m5stack-avatar 的 Expression）
enum class Expression {
  Sleepy,   // 未连接:闭眼缓慢呼吸
  Neutral,  // 已连接空闲:眨眼东张西望
  Happy,    // 会话运行中:专注
  Doubt,    // 有待审批:警觉
  Dizzy,    // 摇晃:螺旋眼
  Love,     // 批准:飘爱心
  Sad,      // 拒绝:委屈
  Angry,    // 会话出错:生气/惊吓
};

// LED 灯效(映射到 Unit RGB)
enum class Led {
  Off,          // 未连接
  CyanDim,      // 空闲
  GreenBreath,  // 运行中
  RedBlink,     // 待审批
  RainbowSpin,  // 摇晃
  PinkPulse,    // 批准
  DarkBlue,     // 拒绝
  YellowFlash,  // 出错
};

// 振动反馈(映射到 M5.Power.setVibration）
enum class Vibration {
  None,
  Buzz,       // 摇晃:嗡一下
  DoubleTap,  // 批准:轻快两下
  Pulse,      // 待审批:周期短震
  LongBuzz,   // 出错:长震
};

// 稳态输入(M1 用按键模拟;M2 用 BLE 的 BuddyState 填充)
struct PetInputs {
  bool connected = false;   // 桥接↔daemon 是否健康
  int runningSessions = 0;  // 运行中会话数
  int pendingApprovals = 0; // 待审批数
};

// 状态机输出
struct PetVisual {
  Expression expression;
  Led led;
  Vibration vibration;
  uint8_t intensity = 0;  // 0..255,冒汗等强度(由 runningSessions 映射)
};

// 瞬态动画时长(ms)
constexpr uint32_t kShakeMs = 2000;
constexpr uint32_t kApproveMs = 1500;
constexpr uint32_t kDenyMs = 1500;
constexpr uint32_t kErrorMs = 3000;
constexpr uint32_t kReliefMs = 1500;
constexpr uint32_t kDrowsyAfterMs = 60000;

/**
 * 宠物状态机。喂稳态输入 + 瞬态事件(带触发时刻),update(now) 计算当前
 * 应显示的 PetVisual。瞬态优先于稳态,并在各自时长后自动回落。
 *
 * 优先级(高→低):
 *   瞬态: Shake > Error > Approve > Deny > Relief   (任一未过期则盖过稳态)
 *   稳态: Disconnected > Pending>0 > Running>0 > Idle
 * 说明:Disconnected 是稳态最高(fail-safe:数据不可信时一律显示未连接);
 *      瞬态可盖过 Disconnected(摇晃是本地 IMU,断连也能玩)。
 *      稳态额外:idle 持续 kDrowsyAfterMs 后转 Drowsy(打盹,显示 Sleepy)。
 *
 * 不依赖任何 Arduino/M5 头文件,可在 host 上单测。
 */
class PetStateMachine {
 public:
  void setInputs(const PetInputs& in) { inputs_ = in; }

  void onShake(uint32_t now_ms) { shakeUntil_ = now_ms + kShakeMs; }
  void onApprove(uint32_t now_ms) { approveUntil_ = now_ms + kApproveMs; }
  void onDeny(uint32_t now_ms) { denyUntil_ = now_ms + kDenyMs; }
  void onSessionError(uint32_t now_ms) { errorUntil_ = now_ms + kErrorMs; }
  void onApprovalsCleared(uint32_t now_ms) { reliefUntil_ = now_ms + kReliefMs; }

  // 计算当前应显示的 visual(处理瞬态衰减)。推进 idle 计时,有副作用,故非 const。
  PetVisual update(uint32_t now_ms);

 private:
  PetInputs inputs_;
  uint32_t shakeUntil_ = 0;
  uint32_t approveUntil_ = 0;
  uint32_t denyUntil_ = 0;
  uint32_t errorUntil_ = 0;
  uint32_t reliefUntil_ = 0;
  uint32_t idleSinceMs_ = 0;
  bool idleTracked_ = false;
};

// 每个表情对应的简短文字标签,显示在 avatar speech bubble 里。
// 因 8 个状态映射到 avatar 仅 6 个内置脸会撞脸(尤其 Dizzy/Doubt),
// 文字标签保证用户始终能区分当前状态。返回静态字符串字面量。
const char* expressionLabel(Expression e);

// M1 演示用:预设互斥场景,中键轮流切换。返回该场景索引(取模)对应的稳态输入。
// idle / busy / pending / disconnected 四个,覆盖 4 个稳态表情。
constexpr int kDemoScenarioCount = 4;
PetInputs demoScenario(int index);

}  // namespace buddy
