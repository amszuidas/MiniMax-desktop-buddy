# M0 → M1 follow-ups (from final review)

Carried forward from the M0 whole-implementation review (2026-06-02). None block
M0; the core approve/deny flow is solid and self-heals via `reconcile()` on every
reconnect. These are robustness / derived-state items to address as M1–M3 build out.

## Important (address in M1, before hardware shows state)
- **Running-session count drifts.** `StateModel.running` is updated only from
  `session.start/finish/error/abort` SSE events. `reconcile()`/`onConnected()`
  resync `pending` but NOT `running`, so: (a) sessions already running when the
  bridge connects are invisible (the common case — daemon outlives the bridge);
  (b) events lost during an SSE drop never resync. The documented daemon contract
  exposes no sessions-snapshot endpoint. Fix: add/locate a running-sessions list
  endpoint and resync `running` alongside `listPending()` in `onConnected()`.
- **No liveness watchdog despite `bodyTimeout:0`/`headersTimeout:0`.** On a true
  half-open socket (laptop sleep/wake, frozen daemon, no FIN/RST) the SSE
  `for await` hangs forever → `onConnectionChange(false)` never fires → `c` stays
  1 against a dead daemon. Use the daemon's 60s heartbeat as an app-level
  watchdog: track last-frame time, `abort()` the AbortController if no frame in
  ~90s so the reconnect loop runs.
- **Structured `toolInput` rendered blank.** `StateModel` keeps `toolInput` only
  when it's a string (state-model.ts), but real tool inputs are often objects
  (e.g. `{command:"ls"}`). Approve/deny still works (keyed by requestId) but the
  human/device sees no context. Decide a rendering rule (summarize JSON, or prefer
  `toolDescription`) before the approval card moves to hardware. Also budget the
  device payload in BYTES not chars (MAX_DESC=40 chars + 3-byte `…` can exceed BLE
  MTU assumptions).

## Minor
- `decide()` treats any non-throwing 200 as success; it ignores `processed`/
  `skipped`. If the daemon 200s but lists the id under `skipped` (race: already
  auto-resolved), the approval vanishes as if approved. Low impact at M0 scale.
- Add an integration test for `DaemonClient.runLoop` against a local `http.Server`
  emitting canned SSE frames (covers reconnect + non-200 + malformed-frame paths
  that are currently only manually validated).

## Nits
- `LocalApproval` (types.ts) is exported but unused; `PendingApproval.createdAt?`
  is never read. Dead surface — remove or use.
- SSE dispatch keys off the JSON envelope `.type` and discards the SSE `event:`
  line. Correct per contract (both carry the type); revisit only if a frame ever
  ships `data:{}` without `type`.
- Fixed 2s reconnect backoff, no jitter/escalation (fine for localhost).
- `allocLocalId` pool-exhaustion/wrap branch (state-model.ts) is untested
  (implausible at >255 concurrent pending, but it's the non-obvious branch).

## Resolved in M0
- ✅ SSE non-200 body leak in `runLoop` (the `bc74a88` drain pattern, applied to
  `/events` too) — fixed post-review.
