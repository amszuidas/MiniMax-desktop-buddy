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

## M0 status (bridge, no hardware)

Implemented and verified:
- `pnpm test` — 22 unit tests (sse-parse, daemon-port, state-model, controller)
- `pnpm ble-probe` — **verified**: `@abandonware/noble` reaches `poweredOn` and
  scans on this Mac (saw 274 BLE peripherals, exit 0). This retires the project's
  biggest risk — the Node+noble (B1) path for BLE transport is viable; no Swift
  fallback needed.
- `pnpm bridge` — connects to a running MiniMax Code daemon, shows live
  running-session / pending-approval state, approves & denies REAL tool calls
  from the terminal (allow once / allow always / deny). *Live daemon round-trip
  is a manual acceptance step — see below.*

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
