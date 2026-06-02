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

## M1 controls (button-simulated state, until BLE lands in M2)
- BtnA: cycle connection (disconnected ↔ connected)
- BtnB: +1 pending approval (tap) / clear pending (long press)
- BtnC: toggle a running session
- Shake the device: dizzy spiral eyes

## M1 status
- `pio test -e native` — pet state machine: 12 unit tests (steady priority,
  transient triggers + decay + fallback, fail-safe, shake-while-disconnected,
  millis-wrap-safe timing). Host-run, no hardware.
- `pio run -e m5stack-core2` — firmware compiles (avatar + buttons + IMU +
  vibration + Unit RGB). Requires `-std=gnu++2a` (designated initializers); set
  in the core2 env.
- 📋 On-device verification (avatar face, button-simulated states, shake→dizzy,
  vibration, RGB) pending the physical Core2 (in shipping).

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
  approximations; bespoke spiral-eyes / floating-hearts come in M4.

## Next: M2 — replace button-simulated inputs with real BLE BuddyState
The state machine input (`buddy::PetInputs`) is already the BLE contract shape
(connected / runningSessions / pendingApprovals). M2 swaps the button simulation
in `loop()` for a BLE GATT peripheral fed by the Mac bridge.
