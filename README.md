# MiniMax Desktop Buddy

Desktop AI-companion hardware for MiniMax Code. See `docs/` (in the MiniMax Code repo)
for the full design spec.

## Packages
- `packages/bridge` — Mac-side bridge: subscribes to the MiniMax Code daemon SSE
  stream, derives Buddy state, relays approvals. (M0: terminal-only, no hardware.)

## Quick start (M0)
```bash
pnpm install
pnpm bridge      # connect to a running daemon, print state, approve via keypress
pnpm ble-probe   # verify @abandonware/noble works on this Mac
```

By default the bridge reads the daemon's port from `~/.mavis/daemon.port` (the
daemon's standard data dir). For a non-default profile (e.g. a worktree daemon
whose data dir is `~/.mavis-<profile>`), point the bridge at it:
```bash
BUDDY_DATA_DIR=~/.mavis-<profile> pnpm bridge   # override data dir
BUDDY_DAEMON_PORT=15321 pnpm bridge             # or pin the port directly
```

## M0 status (bridge, no hardware)

Implemented and verified:
- `pnpm test` — 25 unit tests (sse-parse, daemon-port, state-model, controller)
- `pnpm ble-probe` — **verified**: `@abandonware/noble` reaches `poweredOn` and
  scans on this Mac (saw 274 BLE peripherals, exit 0). This retires the project's
  biggest risk — the Node+noble (B1) path for BLE transport is viable; no Swift
  fallback needed.
- `pnpm bridge` — **verified** end-to-end: resolves the real daemon port from
  `~/.mavis/daemon.port`, connects over SSE, lists pending approvals, and
  approves & denies REAL tool calls from the terminal (allow once / allow
  always / deny).

> **Known M0 limitation:** the running-session count (`r`) is derived purely from
> SSE lifecycle events, so sessions already running when the bridge connects are
> not counted, and the count can drift across SSE drops. The pending-approval
> path resyncs on every reconnect and is unaffected. See `docs/M0-followups.md`.

### Manual acceptance: live approve/deny round-trip
With a MiniMax Code daemon running on this Mac:
1. `pnpm bridge` → the box should show `daemon: CONNECTED` within ~2s.
2. In MiniMax Code, trigger a tool call that needs approval (e.g. a `bash`
   command). The box flips to `▶ APPROVAL #1 [bash] …`.
3. Press `a` (allow once). The agent proceeds and the box returns to
   `(no pending approvals)`.

Next milestones (separate plans):
- M1 firmware "pet hello" on M5Stack Core2 (avatar + buttons + IMU + RGB + vibration)
- M2 BLE GATT transport (device = peripheral, Mac = central)
- M3 integration (real daemon state → pet animation; buttons → real approvals)
