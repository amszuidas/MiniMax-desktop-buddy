# MiniMax Desktop Buddy — Firmware (M5Stack Core2)

The pet that lives on the device. M1 milestone: avatar + buttons + IMU shake +
RGB + vibration, driven by a pure state machine. BLE input comes in M2.

## Layout
- `lib/pet_state/` — pure C++ state machine (no Arduino deps). Maps inputs
  (connected / running / pending + shake/approve/deny/error transients) to a
  visual (expression + LED + vibration). Unit-tested on the host.
- `src/main.cpp`  — hardware adapter: renders the visual via M5Unified +
  m5stack-avatar; reads buttons/IMU into inputs.

## Build & test
```bash
# Host unit tests (no hardware needed):
pio test -e native

# Compile the firmware (installs the ESP32 toolchain; no board needed):
pio run -e m5stack-core2

# Flash to a connected Core2 and watch serial:
pio run -e m5stack-core2 -t upload -t monitor
```

## M1 controls (demo interaction, until BLE lands in M2)
Each state shows a face **and** a speech-bubble label (bottom-right) so states
stay distinguishable even where two states share a built-in avatar face.
- **BtnB (center): one-button scenario cycle** — `idle` → `busy` → `approve?`
  → `zzz` (disconnected) → back to `idle`. Mutually exclusive, no stacking.
- **BtnA (left): approve animation** — `approved` (heart) + double buzz.
- **BtnC (right): deny animation** — `denied` (sad).
- **Shake the device**: `dizzy` + buzz.
Together these cover all 8 faces except `error` (Angry), which only fires from a
real daemon `session.error` event in M2.

## M1 status — ✅ verified on real hardware (Core2 v1.3, 2026-06-04)
- `pio test -e native` — pet state machine: 19 unit tests (steady priority,
  transient triggers + decay + fallback, fail-safe, shake-while-disconnected,
  millis-wrap-safe timing, per-state labels, demo-scenario cycle). Host-run.
- `pio run -e m5stack-core2` — firmware compiles. Requires `-std=gnu++2a`
  (designated initializers); set in the core2 env.
- **On-device: all 8 faces + labels, scenario cycle, approve/deny, shake, and
  vibration confirmed on a physical Core2.** RGB effects require the Unit RGB
  module (Grove Port A); verify when that module is attached.

## Hardware checklist before on-device test
- M5Stack Core2 (v1.1 / v1.3 — both fine; firmware uses M5Unified power
  abstraction so AXP192/AXP2101 differences are handled automatically)
- M5Stack Unit RGB → Grove Port A (only needed for the LED effects)

## Known M1 trade-offs (see commits / M2 plan)
- Vibration uses blocking `delay()` (≤400ms); during a buzz the loop skips
  button/IMU polling. M2 moves to non-blocking vibration.
- `Vibration::Pulse` (pending) fires once on entry, not periodically — periodic
  reminder is an M4 polish item.
- `Dizzy`/`Love` use the nearest built-in avatar faces (Doubt/Happy) as interim
  approximations; the speech-bubble label disambiguates them. Bespoke
  spiral-eyes / floating-hearts come in M4.
- Speech bubble position is fixed bottom-right by the m5stack-avatar library
  (`Balloon.h` hard-codes cx=240/cy=220); not configurable without forking.
- `error` (Angry) face has **no trigger source in M1** — it only fires from a
  real daemon `session.error` event in M2/M3. (`deny`/`Sad` is now reachable via
  BtnC.) See `M1-followups.md`.

## Next: M2 — replace button-simulated inputs with real BLE BuddyState
The state machine input (`buddy::PetInputs`) is already the BLE contract shape
(connected / runningSessions / pendingApprovals). M2 swaps the button simulation
in `loop()` for a BLE GATT peripheral fed by the Mac bridge.
