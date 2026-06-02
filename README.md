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
