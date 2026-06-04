# M3 → 后续 follow-ups

M3 BLE 加固真板验证结果(2026-06-04,Core2 v1.3):

| 加固点 | 结果 |
|---|---|
| 加密配对(Just Works bonding + 加密特征) | ✅ PASS |
| BLE 重连重推状态(I-1) | ✅ PASS |
| 审批回路(改 indication 带确认) | ✅ PASS |
| session.error → Angry 脸 | ⏭️ 跳过(daemon 难触发真实 error;代码+native 测试就绪,待自然 error 时观察) |
| running 重同步 | ❌ 证伪后回滚(见下) |

## running 重同步:此路不通,已回滚(commit 10aa600)
**根因(真板 + 真实接口证实)**:
- `GET /api/agent` 返回顶层数组 `[{name},...]`(非 `{agents:[]}`)——我原代码取 `.agents` 永远空。
- 更致命:`GET /api/agent/:name/session` 返回的 `status.type` 是 **DB 持久化值,永远 finished**,不含"实时运行中"信号。daemon 的"运行中"真相只在内存 `hasActiveTurn`,`getEffectiveSessionStatus()` 内部合并,**无任何外部 HTTP 暴露**(`task-status` 也返回 finished)。
- 结论:用列表接口聚合 running 永远数 0。已删除聚合代码(countRunningSessions/getRunningCount/setRunningCount/runningBaseline/syncRunning/定时器)。

**running 现状**:纯 SSE `session.start`/`session.finish` 事件驱动。**连接后启动的会话准确;桥接连接前就已在跑的会话漏计数**(与 M2 同)。这是当前架构(buddy 只依赖稳定 HTTP/SSE、不改 daemon)下的硬限制。

**彻底修复的唯一路**(留待将来,需跨仓库):在 daemon 加一个真正暴露"实时运行中会话数/列表"的 HTTP 接口(数据源 = 内存 hasActiveTurn / SessionState),走 agent-archon 仓库 worktree+MR 完整流程。届时 buddy 侧恢复一个 onConnect/定时拉取即可。

**教训**:Plan 4 调研时,调研报告其实警告过"列表接口返回 DB status、实时靠内存、无跨 agent 接口",但设计阶段未充分重视,且写了用错误响应形状 mock 的单测(假性通过)。外部接口形状必须打真实接口验证,不能靠 mock 断言。

## Event indication 加密(评审 Important,待真板安全验证)
NimBLE 无 INDICATE_ENC 标志;审批事件(设备→Mac indication)的加密**继承自 bonded link**(Mac 先 WRITE_ENC 写 State 触发配对 → 链路加密后 indication 才流动),非 per-characteristic 强制。READ_ENC 仅护直接 ATT Read。
**待验证**(需手机 BLE 调试 App 如 nRF Connect):未绑定的第二客户端连上后订阅 Event CCCD、按设备键,确认收**不到**明文审批事件。当前威胁模型(个人桌面玩具、payload 低价值、Mac 正常流程先 bond)下风险低,但安全敏感路径应实测。

## 继承自 M2 的未做项(仍适用)
- BLE 设备白名单(NimBLE 1.4 whitelist 对已绑定 peer 有已知缺陷;当前靠 bonding+加密特征)。
- noble 重连无 backoff;stop() 不主动断开 peripheral(SIGINT→exit 覆盖)。
- 非阻塞振动;自绘螺旋眼/爱心特效;周期审批提醒(M4)。
