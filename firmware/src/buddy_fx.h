#pragma once
#ifndef UNIT_TEST
#include <cstdint>
#include "pet_state.h"

// Shared effect state between main loop (writer) and avatar render thread
// (reader).  Single-writer / single-reader on POD fields -- a torn read only
// causes a single-frame glitch, which is harmless (same model as
// ble_peripheral's latestState).
struct BuddyFx {
  buddy::Expression expr = buddy::Expression::Neutral;
  uint8_t intensity = 0;   // sweat intensity 0..255 (mapped from runningSessions)
  uint32_t nowMs = 0;      // current millis(), for animation phase
};

// Global singleton (lifetime = program lifetime; Drawables hold a reference).
inline BuddyFx g_buddyFx;
#endif
