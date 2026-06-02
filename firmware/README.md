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
