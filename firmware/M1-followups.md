# M1 → M2 follow-ups（来自最终整体评审）

M1 固件「宠物 hello」已通过 native 测试 + 编译验证,作为 M2 的地基。以下是整体
评审发现、需在 M2 正视的集成项。均不阻塞 M1。

## 必须在 M2 处理
- **deny/error 瞬态在真板上无触发源(I-1)。** lib 状态机实现并单测了 onDeny /
  onSessionError,但 src/main.cpp 只 wire 了 onShake(IMU)和 onApprove(BtnB 长按)。
  后果:真板上 Sad / Angry 两张脸、DarkBlue / YellowFlash 灯效、LongBuzz 振动在 M1
  永远不触发。它们将由 M2/M3 的真实 daemon 事件驱动,届时自然打通。

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
- BtnB 长按在无 pending 时也会放 Love 爱心(M1 模拟操作的副作用)。
