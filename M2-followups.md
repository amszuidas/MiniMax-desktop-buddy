# M2 → M3 follow-ups(BLE 可靠性)

M2 BLE 双向已打通(协议/适配器 native 测试 + 链路/入口 typecheck)。以下是评审发现、留给 M3 加固的可靠性项。均不阻塞 M2 单设备开发验证。

## Mac 端 noble 链路(`packages/bridge/src/ble-link.ts`)
- **stop() 不主动断开已连接 peripheral。** 当前仅设 stopped 标志 + stopScanning;SIGINT→process.exit 时 OS 会拆 CoreBluetooth 会话,所以现在无害。但若将来有"优雅关闭不退进程"的生命周期,连接会残留。修法:存 peripheral 引用,stop() 里 `await peripheral.disconnectAsync()`。
- **重连无 backoff。** connect 失败 / disconnect 后立即 `startScanningAsync`,无延迟。若 connectAsync 持续失败(正是 noble.reset 针对的 macOS 残留态),会忙重连。修法:重连前加递增 backoff(如 1s→2s→5s 上限)。
- 已做(M2):connect 前 `noble.reset?.()` 防 macOS 残留态挂死;stop-before-connect;once('disconnect');guarded rescan。

## 安全(用户本轮明确推迟到 M3)
- BLE bonding + 设备白名单:当前按广播名 'MmxBuddy' 连第一个匹配的,未配对。M3 加 bonding,桥接只连已绑定身份。审批是安全敏感动作,长期必须加固。

## 设备侧 session.error → Angry 脸
- M2 只传 state + approve/deny/always 事件。session.error 需要 M0 bridge 先在 BuddyState/事件里显式表示 error(见 firmware/M1-followups.md I-2),Angry 脸才有真实数据源。
