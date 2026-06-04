# M2 → M3 follow-ups(BLE 可靠性)

M2 BLE 双向已打通并**真板端到端验证通过**(Mac 连上设备、宠物反映真实状态、按键真实批准/拒绝 daemon 工具调用)。以下是评审发现、留给 M3 加固的项。均不阻塞 M2 核心回路(都在安全方向:显示更少,绝不误批准)。

## M3 最高优先(最终整体评审发现,真板"主动使用"掩盖了)
- **I-1:BLE (重)连后不主动重推状态。** writeState 只由 Controller 的 onState 触发(daemon 事件/连接变化),BleLink 无 onConnect 钩子。SSE(localhost)几乎总比 BLE 先就绪,首次 onConnected 推送在 stateChar 还是 null 时被丢弃(writeState 静默 no-op);心跳事件在 emit 前被过滤,所以**空闲 daemon + Mac 睡醒/设备超距回来后,宠物会卡在 zzz,直到下个非心跳事件**。修法:BleLink 加 `onConnect(cb)`,bridge-ble.ts 里 onConnect → 重推 `controller.state()`(已暴露)。便宜、高价值。
- **I-2:无显式 MTU 协商/守卫。** 默认 ATT MTU(23→20 可用字节)下,`{"ev":"approve","id":7}`(23B)/`always`(22B)/`approve id≥100`(25B)会超单次 notify 被截断 → decodeEvent 返回 null → **按键静默丢失**;只有 `deny`(20B)能过。173B 的 State 写同理需大 MTU。真板能用是因 macOS/CoreBluetooth↔NimBLE 默认协商 ~185B,但无代码保证。且 notify 不带确认,丢失的审批只有乐观 UI(main.cpp 先播 Love)→ 用户以为批了其实没批(~1.5s 后自愈回 approve?)。修法:设备 `NimBLEDevice::setMTU(247)` + Mac 端日志协商到的 MTU;Event 特征改 **indication(带确认)**,丢失可重试。审批是安全敏感动作,M3 必做。

## Mac 端 noble 链路(`packages/bridge/src/ble-link.ts`)
- **stop() 不主动断开已连接 peripheral。** 当前仅设 stopped 标志 + stopScanning;SIGINT→process.exit 时 OS 会拆 CoreBluetooth 会话,所以现在无害。但若将来有"优雅关闭不退进程"的生命周期,连接会残留。修法:存 peripheral 引用,stop() 里 `await peripheral.disconnectAsync()`。
- **重连无 backoff。** connect 失败 / disconnect 后立即 `startScanningAsync`,无延迟。若 connectAsync 持续失败(正是 noble.reset 针对的 macOS 残留态),会忙重连。修法:重连前加递增 backoff(如 1s→2s→5s 上限)。
- 已做(M2):connect 前 `noble.reset?.()` 防 macOS 残留态挂死;stop-before-connect;once('disconnect');guarded rescan。

## 状态同步的诚实边界(评审纠正了 README 的轻微夸大)
- **running 计数不在连接时重同步(继承自 M0)。** 桥接连上前就已运行的会话显示为 idle 而非 busy;README 说"reflects live running-session count"仅对连接后启动的会话成立。M3 需加"列运行中会话"接口在 onConnected 时重同步(见 M0-followups)。
- **带外解决不及时反映。** StateModel.applyEvent 只处理 permission.ask 新增,不移除被其他客户端解决的待审批 → 设备显示幽灵审批,直到被按(no-op re-reply 自愈)或重连 reconcile。M3 处理 daemon 的 permission.resolved/answered 事件可即时清除。
- **设备不读 v / a.s。** parseState 不消费协议版本 v(M3 改契约会被静默误解析——加版本检查)和会话标签 a.s(传了但没用,浪费 MTU 预算)。

## 安全(用户本轮明确推迟到 M3)
- BLE bonding + 设备白名单:当前按广播名 'MmxBuddy' 连第一个匹配的,未配对。M3 加 bonding,桥接只连已绑定身份。审批是安全敏感动作,长期必须加固。
- `latestState()` torn-read:onWrite(BLE 任务)与 latestState(主循环)非原子拷贝;唯一有安全含义的是 approvalId(撕裂值若恰等于另一有效 localId 可能批错,概率 ~1e-7,decide 对垃圾 id no-op)。M3 收紧时让 id 读一致即可。

## 设备侧 session.error → Angry 脸
- M2 只传 state + approve/deny/always 事件。session.error 需要 M0 bridge 先在 BuddyState/事件里显式表示 error(见 firmware/M1-followups.md I-2),Angry 脸才有真实数据源。
