# M1 → M2 follow-ups（来自最终整体评审）

M1 固件「宠物 hello」已通过 native 测试 + 编译验证,作为 M2 的地基。以下是整体
评审发现、需在 M2 正视的集成项。均不阻塞 M1。

## 必须在 M2 处理
- **error 瞬态在真板上仍无触发源(I-1,部分已解)。** lib 状态机实现并单测了
  onDeny / onSessionError。**真板调试后**:demo 交互已把 BtnA→onApprove、BtnC→onDeny
  接上,所以 approve/deny(Love/Sad)真板可触发了。**仅剩 onSessionError(Angry/error)**
  无触发源——它将由 M2/M3 的真实 daemon `session.error` 事件驱动,届时自然打通。

- **瞬态需要专门的 BLE 通道,不能从计数快照差分(I-2)。** 状态机有两类输入:
  setInputs(level:connected/running/pending)和 onApprove/onDeny/onSessionError(edge)。
  M0 的 BuddyState{v,c,r,p,a} 只是计数快照:
  - 稳态对齐干净:c→connected, r→runningSessions, p→pendingApprovals(M2 换源即可)。
  - 瞬态载不动:approve vs deny 无法从"p 减少"区分;session.error 在 BuddyState 里
    完全没有表示(M0 bridge 把它和正常 finish 一样只做 running--)。
  **M2 行动**:BLE GATT 拆两个特征——state 快照喂 setInputs,transient notify 携带
  {approve|deny|error} 驱动对应 onXxx。同时给 M0 bridge 合约补 session.error 的显式
  表示。onShake 保持本地 IMU,不进 BLE。lib API 无需改动(边界设计已就位)。

## 可选打磨(低优先)
- Dizzy/Love 目前用内置 avatar 表情(Doubt/Happy)近似;M4 做自绘螺旋眼/爱心叠加。
- Vibration::Pulse(待审批)仅进入时震一次;周期提醒留 M4。
- 振动用阻塞 delay()(≤400ms),期间丢按键/IMU;M2 改非阻塞。
- ShakeDetector 开机头 1500ms 冷却(lastTrigger_=0 所致),无害。

## 真板调试经验(2026-06-04,Core2 v1.3 首次上板)
- **avatar 仅 6 个内置脸,8 状态必然撞脸。** 最坑:Dizzy 当时映射成 Doubt,与
  "待审批"同为流汗脸 → 误判。解法:每状态加 speech-bubble 文字标签(idle/busy/
  approve?/dizzy/approved/denied/error/zzz),撞脸也能区分。M4 自绘特效可彻底解决。
- **气泡位置库内写死右下角**(Balloon.h cx=240/cy=220),不可配置(除非 fork)。
- **独立 toggle 按键会状态叠加、误导。** 初版 BtnA/B/C 各切一个布尔,pending 累加
  只能长按清零 → 脸卡死遮蔽其它,让人误以为"按键无效"(实为按键完全正常,诊断屏
  证明 clickA/B/C 计数 + 触摸坐标分区 0-30/120-150/240-290 全对)。改为"中键一键轮流
  互斥场景"后演示清晰。**教训**:无真板写的交互固件,按键语义要等真板验证;纯逻辑
  (状态机)可 native 测先行,交互层留到上板。
- **诊断手段**:临时刷一版纯文本固件(把按键计数/触摸坐标/状态机输出打屏)比靠用户
  描述屏幕快得多。下次真板排查可复用这一招(独立子工程 + lib_extra_dirs 引用上层 lib)。
- **M5Unified 钉版本注意**:core2 env 钉 M5Unified@0.2.16,但其 library.json 要求
  >=0.2.22;当前能编过,M2 若遇库解析问题先查这里。
