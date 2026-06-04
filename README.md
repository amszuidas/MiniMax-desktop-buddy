# MiniMax Desktop Buddy

Desktop AI-companion hardware for MiniMax Code: an M5Stack Core2 shows a pet that
reflects your live MiniMax Code session/approval state over BLE, and its buttons
**approve or deny real tool calls** on the daemon. See `docs/` (in the MiniMax
Code repo) for the full design spec.

**Status:** M0 (Mac bridge) ✅ · M1 (Core2 firmware) ✅ · M2 (BLE bidirectional)
✅ — all verified on real hardware. The core product works end-to-end.

## Packages
- `packages/bridge` — Mac-side bridge (Node/TS): subscribes to the MiniMax Code
  daemon SSE stream, derives Buddy state, relays it to the device over BLE, and
  turns device button presses into real `batch-reply` approval decisions.
- `firmware/` — M5Stack Core2 firmware (PlatformIO): NimBLE peripheral + pet
  state machine + avatar/IMU/vibration/RGB. See `firmware/README.md`.

## Quick start
```bash
pnpm install

# M2 — full product: pet driven by real daemon state over BLE, buttons approve/deny.
# Requires a flashed Core2 (see firmware/README.md) + a running MiniMax Code daemon.
pnpm --filter @buddy/bridge bridge:ble

# M0 — terminal-only bridge (no hardware): approve/deny from the keyboard.
pnpm --filter @buddy/bridge bridge

# verify @abandonware/noble works on this Mac
pnpm --filter @buddy/bridge ble-probe
```

By default the bridge reads the daemon's port from `~/.mavis/daemon.port` (the
daemon's standard data dir). For a non-default profile (e.g. a worktree daemon
whose data dir is `~/.mavis-<profile>`), point the bridge at it:
```bash
BUDDY_DATA_DIR=~/.mavis-<profile> pnpm --filter @buddy/bridge bridge:ble
BUDDY_DAEMON_PORT=15321 pnpm --filter @buddy/bridge bridge:ble
```

## Milestones
- **M0 — Mac bridge** ✅ verified: resolves the daemon port, connects over SSE,
  lists pending approvals, approves/denies REAL tool calls from the terminal.
- **M1 — Core2 firmware "pet hello"** ✅ verified on hardware: pet state machine
  (native-tested) + avatar/buttons/IMU/vibration/RGB. See `firmware/`.
- **M2 — BLE bidirectional** ✅ verified on hardware: device = NimBLE peripheral,
  Mac = noble central; daemon state → pet over BLE, device buttons → real
  approvals. See `firmware/README.md` + `M2-followups.md`.
- **M3 / M4 (next)** — harden BLE (bonding/whitelist, reconnect backoff,
  session.error→error face); bespoke pet animations; non-blocking vibration.
