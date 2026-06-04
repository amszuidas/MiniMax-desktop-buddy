# Expression Animation Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current black-and-white buddy render style with colorful procedural expression animations while keeping the same 8 expressions and trigger behavior.

**Architecture:** Keep `PetStateMachine` and BLE unchanged. Add a small pure palette library, then use Avatar `ColorPalette` for per-expression base colors and the existing custom drawable seams (`BuddyEffect`, custom eyes, `driveFace`) for colorful backgrounds, glows, symbols, and motion.

**Tech Stack:** M5Stack Core2 firmware, PlatformIO, Arduino, M5Unified, m5stack-avatar 0.10.0, FastLED, Unity native tests.

---

## Scope Check

This plan covers one subsystem: the firmware render layer. It does not change the bridge, BLE protocol, approval event mapping, state priorities, or expression count.

## File Structure

- Create `firmware/lib/buddy_render/buddy_palette.h`
  - Pure C++ render palette and small color helpers.
  - No Arduino or M5 headers, so it can run in native tests.

- Create `firmware/test/test_buddy_render/test_buddy_palette.cpp`
  - Unity tests for RGB565 conversion, palette distinctness, pulse math, and workload sweat count.

- Create `firmware/src/buddy_avatar_palette.h`
  - M5/avatar adapter that converts a `buddy_render::BuddyPalette` into `m5avatar::ColorPalette`.
  - Called from `main.cpp` only when the expression changes, avoiding per-frame `ColorPalette::set()` log spam.

- Modify `firmware/src/main.cpp`
  - Include and call `applyBuddyPalette()` before updating avatar expression and speech label.
  - Preserve all BLE, button, IMU, LED, and vibration behavior.

- Modify `firmware/src/buddy_effect.h`
  - Draw a colorful mood layer behind the mouth and eyes.
  - Recolor existing hearts, sweat, Zzz, stars, alert, tear, and anger symbols.

- Modify `firmware/src/spiral_eye.h`
- Modify `firmware/src/heart_eye.h`
- Modify `firmware/src/alert_eye.h`
- Modify `firmware/src/busy_eye.h`
- Modify `firmware/src/sad_eye.h`
- Modify `firmware/src/angry_eye.h`
  - Replace one-color drawing with palette primary/accent/background colors.

- Modify `firmware/src/face_drive.h`
  - Tune per-expression motion so the new keyframes feel distinct without changing state durations.

## Task 1: Add Pure Palette Library

**Files:**
- Create: `firmware/lib/buddy_render/buddy_palette.h`
- Create: `firmware/test/test_buddy_render/test_buddy_palette.cpp`

- [ ] **Step 1: Write the failing palette test**

Create `firmware/test/test_buddy_render/test_buddy_palette.cpp`:

```cpp
#include <unity.h>
#include "buddy_palette.h"

using buddy::Expression;
using buddy_render::BuddyPalette;
using buddy_render::blend565;
using buddy_render::paletteFor;
using buddy_render::rgb565;
using buddy_render::sweatCountForIntensity;
using buddy_render::triWave8;

void test_rgb565_known_values() {
  TEST_ASSERT_EQUAL_UINT16(0x0000, rgb565(0, 0, 0));
  TEST_ASSERT_EQUAL_UINT16(0xFFFF, rgb565(255, 255, 255));
  TEST_ASSERT_EQUAL_UINT16(0xF800, rgb565(255, 0, 0));
  TEST_ASSERT_EQUAL_UINT16(0x07E0, rgb565(0, 255, 0));
  TEST_ASSERT_EQUAL_UINT16(0x001F, rgb565(0, 0, 255));
}

void test_all_expression_backgrounds_are_colorful() {
  const Expression expressions[] = {
      Expression::Sleepy, Expression::Neutral, Expression::Happy,
      Expression::Doubt, Expression::Dizzy, Expression::Love,
      Expression::Sad, Expression::Angry,
  };
  for (Expression e : expressions) {
    BuddyPalette p = paletteFor(e);
    TEST_ASSERT_NOT_EQUAL_UINT16(0x0000, p.background);
    TEST_ASSERT_NOT_EQUAL_UINT16(p.background, p.primary);
    TEST_ASSERT_NOT_EQUAL_UINT16(p.background, p.accent);
  }
}

void test_pending_and_error_palettes_are_distinct() {
  BuddyPalette pending = paletteFor(Expression::Doubt);
  BuddyPalette error = paletteFor(Expression::Angry);
  TEST_ASSERT_NOT_EQUAL_UINT16(pending.background, error.background);
  TEST_ASSERT_NOT_EQUAL_UINT16(pending.accent, error.accent);
  TEST_ASSERT_NOT_EQUAL_UINT16(pending.glow, error.glow);
}

void test_tri_wave_reaches_edges_and_midpoint() {
  TEST_ASSERT_EQUAL_UINT8(0, triWave8(0, 1000));
  TEST_ASSERT_EQUAL_UINT8(127, triWave8(250, 1000));
  TEST_ASSERT_EQUAL_UINT8(255, triWave8(500, 1000));
  TEST_ASSERT_EQUAL_UINT8(128, triWave8(750, 1000));
  TEST_ASSERT_EQUAL_UINT8(0, triWave8(1000, 1000));
}

void test_blend565_endpoints() {
  uint16_t red = rgb565(255, 0, 0);
  uint16_t blue = rgb565(0, 0, 255);
  TEST_ASSERT_EQUAL_UINT16(red, blend565(red, blue, 0));
  TEST_ASSERT_EQUAL_UINT16(blue, blend565(red, blue, 255));
}

void test_sweat_count_for_workload_intensity() {
  TEST_ASSERT_EQUAL(1, sweatCountForIntensity(0));
  TEST_ASSERT_EQUAL(1, sweatCountForIntensity(80));
  TEST_ASSERT_EQUAL(2, sweatCountForIntensity(160));
  TEST_ASSERT_EQUAL(3, sweatCountForIntensity(240));
  TEST_ASSERT_EQUAL(3, sweatCountForIntensity(255));
}

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_rgb565_known_values);
  RUN_TEST(test_all_expression_backgrounds_are_colorful);
  RUN_TEST(test_pending_and_error_palettes_are_distinct);
  RUN_TEST(test_tri_wave_reaches_edges_and_midpoint);
  RUN_TEST(test_blend565_endpoints);
  RUN_TEST(test_sweat_count_for_workload_intensity);
  return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:

```bash
pio test -e native -f test_buddy_render
```

Expected: FAIL during compilation with an include error for `buddy_palette.h`.

- [ ] **Step 3: Add the palette header**

Create `firmware/lib/buddy_render/buddy_palette.h`:

```cpp
#pragma once
#include <cstdint>
#include "pet_state.h"

namespace buddy_render {

struct BuddyPalette {
  uint16_t background;
  uint16_t backgroundAlt;
  uint16_t face;
  uint16_t primary;
  uint16_t secondary;
  uint16_t accent;
  uint16_t accent2;
  uint16_t glow;
  uint16_t balloonBg;
  uint16_t balloonFg;
};

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) |
                               ((g & 0xF8) << 3) |
                               (b >> 3));
}

constexpr uint8_t red8(uint16_t c) {
  return static_cast<uint8_t>(((c >> 11) & 0x1F) * 255 / 31);
}

constexpr uint8_t green8(uint16_t c) {
  return static_cast<uint8_t>(((c >> 5) & 0x3F) * 255 / 63);
}

constexpr uint8_t blue8(uint16_t c) {
  return static_cast<uint8_t>((c & 0x1F) * 255 / 31);
}

inline uint16_t blend565(uint16_t a, uint16_t b, uint8_t t) {
  const uint16_t inv = static_cast<uint16_t>(255 - t);
  uint8_t r = static_cast<uint8_t>((red8(a) * inv + red8(b) * t) / 255);
  uint8_t g = static_cast<uint8_t>((green8(a) * inv + green8(b) * t) / 255);
  uint8_t bl = static_cast<uint8_t>((blue8(a) * inv + blue8(b) * t) / 255);
  return rgb565(r, g, bl);
}

inline uint8_t triWave8(uint32_t nowMs, uint16_t periodMs) {
  if (periodMs == 0) return 0;
  uint32_t p = nowMs % periodMs;
  uint32_t half = periodMs / 2;
  if (half == 0) return 0;
  if (p <= half) return static_cast<uint8_t>((p * 255) / half);
  return static_cast<uint8_t>(((periodMs - p) * 255) / half);
}

inline int sweatCountForIntensity(uint8_t intensity) {
  if (intensity >= 240) return 3;
  if (intensity >= 160) return 2;
  return 1;
}

inline BuddyPalette paletteFor(buddy::Expression e) {
  switch (e) {
    case buddy::Expression::Sleepy:
      return {rgb565(18, 22, 62), rgb565(37, 50, 111), rgb565(59, 74, 143),
              rgb565(214, 229, 255), rgb565(120, 162, 255),
              rgb565(147, 189, 255), rgb565(78, 101, 195),
              rgb565(53, 83, 175), rgb565(225, 235, 255), rgb565(21, 26, 69)};
    case buddy::Expression::Neutral:
      return {rgb565(210, 248, 239), rgb565(247, 252, 238), rgb565(255, 250, 224),
              rgb565(28, 65, 72), rgb565(72, 139, 128),
              rgb565(26, 188, 156), rgb565(255, 214, 125),
              rgb565(135, 229, 206), rgb565(255, 255, 245), rgb565(32, 58, 63)};
    case buddy::Expression::Happy:
      return {rgb565(29, 171, 128), rgb565(42, 217, 203), rgb565(231, 255, 224),
              rgb565(14, 72, 67), rgb565(10, 137, 122),
              rgb565(132, 255, 226), rgb565(255, 234, 99),
              rgb565(96, 241, 177), rgb565(235, 255, 246), rgb565(10, 77, 67)};
    case buddy::Expression::Doubt:
      return {rgb565(244, 91, 91), rgb565(255, 152, 96), rgb565(255, 239, 202),
              rgb565(91, 28, 39), rgb565(155, 51, 55),
              rgb565(255, 226, 84), rgb565(255, 255, 220),
              rgb565(255, 191, 91), rgb565(255, 250, 225), rgb565(91, 28, 39)};
    case buddy::Expression::Dizzy:
      return {rgb565(89, 54, 196), rgb565(167, 89, 226), rgb565(244, 229, 255),
              rgb565(48, 25, 92), rgb565(105, 73, 180),
              rgb565(255, 221, 83), rgb565(114, 239, 255),
              rgb565(214, 154, 255), rgb565(248, 238, 255), rgb565(54, 33, 112)};
    case buddy::Expression::Love:
      return {rgb565(255, 96, 166), rgb565(255, 156, 207), rgb565(255, 237, 247),
              rgb565(108, 22, 74), rgb565(173, 41, 108),
              rgb565(255, 44, 126), rgb565(255, 227, 80),
              rgb565(255, 185, 218), rgb565(255, 245, 250), rgb565(102, 20, 74)};
    case buddy::Expression::Sad:
      return {rgb565(54, 103, 196), rgb565(104, 151, 225), rgb565(225, 238, 255),
              rgb565(24, 50, 94), rgb565(63, 102, 159),
              rgb565(159, 220, 255), rgb565(196, 176, 255),
              rgb565(126, 180, 240), rgb565(237, 246, 255), rgb565(24, 50, 94)};
    case buddy::Expression::Angry:
      return {rgb565(224, 49, 45), rgb565(255, 115, 48), rgb565(255, 229, 201),
              rgb565(76, 17, 17), rgb565(126, 26, 22),
              rgb565(255, 216, 69), rgb565(255, 255, 230),
              rgb565(255, 142, 67), rgb565(255, 238, 214), rgb565(76, 17, 17)};
  }
  return paletteFor(buddy::Expression::Neutral);
}

}  // namespace buddy_render
```

- [ ] **Step 4: Run palette tests**

Run:

```bash
pio test -e native -f test_buddy_render
```

Expected: PASS, 6 tests.

- [ ] **Step 5: Commit palette library**

Run:

```bash
git add firmware/lib/buddy_render/buddy_palette.h firmware/test/test_buddy_render/test_buddy_palette.cpp
git commit -m "feat(firmware): add buddy render palettes"
```

Expected: commit succeeds.

## Task 2: Apply Avatar Palette Per Expression

**Files:**
- Create: `firmware/src/buddy_avatar_palette.h`
- Modify: `firmware/src/main.cpp`

- [ ] **Step 1: Add the avatar palette adapter**

Create `firmware/src/buddy_avatar_palette.h`:

```cpp
#pragma once
#ifndef UNIT_TEST
#include <Avatar.h>
#include <ColorPalette.h>
#include "buddy_palette.h"

namespace buddy_face {

inline void applyBuddyPalette(m5avatar::Avatar& avatar, buddy::Expression expr) {
  buddy_render::BuddyPalette p = buddy_render::paletteFor(expr);
  m5avatar::ColorPalette cp = avatar.getColorPalette();
  cp.set(COLOR_BACKGROUND, p.background);
  cp.set(COLOR_PRIMARY, p.primary);
  cp.set(COLOR_SECONDARY, p.secondary);
  cp.set(COLOR_BALLOON_BACKGROUND, p.balloonBg);
  cp.set(COLOR_BALLOON_FOREGROUND, p.balloonFg);
  avatar.setColorPalette(cp);
}

}  // namespace buddy_face
#endif
```

- [ ] **Step 2: Include the adapter in `main.cpp`**

Modify the include block in `firmware/src/main.cpp`:

```cpp
#include "buddy_face.h"
#include "buddy_avatar_palette.h"
#include "buddy_fx.h"
```

- [ ] **Step 3: Track palette initialization in `main.cpp`**

Add this global beside `lastExpression`:

```cpp
bool paletteInitialized = false;
```

- [ ] **Step 4: Apply palette when expression changes**

Replace the expression-change block in `firmware/src/main.cpp`:

```cpp
  if (!paletteInitialized || v.expression != lastExpression) {
    applyBuddyPalette(avatar, v.expression);
    avatar.setExpression(baseLibExpression(v.expression));
    avatar.setSpeechText(buddy::expressionLabel(v.expression));
    lastExpression = v.expression;
    paletteInitialized = true;
  }
```

- [ ] **Step 5: Compile firmware**

Run:

```bash
pio run -e m5stack-core2
```

Expected: build succeeds.

- [ ] **Step 6: Commit palette adapter**

Run:

```bash
git add firmware/src/buddy_avatar_palette.h firmware/src/main.cpp
git commit -m "feat(firmware): apply expression color palettes"
```

Expected: commit succeeds.

## Task 3: Add Mood Backdrop And Colorful Overlay Effects

**Files:**
- Modify: `firmware/src/buddy_effect.h`

- [ ] **Step 1: Include the palette header**

Modify the include block in `firmware/src/buddy_effect.h`:

```cpp
#include <cmath>
#include <Drawable.h>
#include <Mouth.h>
#include "buddy_fx.h"
#include "buddy_palette.h"
```

- [ ] **Step 2: Change draw order to render atmosphere before the mouth**

Replace the beginning of `BuddyEffect::draw()` with:

```cpp
  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    buddy_render::BuddyPalette p = buddy_render::paletteFor(g_buddyFx.expr);
    uint32_t now = g_buddyFx.nowMs;

    drawMoodBackdrop(spi, p, g_buddyFx.expr, now, g_buddyFx.intensity);

    if (mouth_) {
      mouth_->draw(spi, rect, ctx);
    }

    switch (g_buddyFx.expr) {
      case buddy::Expression::Love:
        drawFloatingHearts(spi, p.accent, p.accent2, now);
        break;
      case buddy::Expression::Happy:
        drawSweatDrops(spi, p.accent, now, g_buddyFx.intensity);
        break;
      case buddy::Expression::Sleepy:
        drawZzz(spi, p.accent, now);
        break;
      case buddy::Expression::Dizzy:
        drawDizzyStars(spi, p.accent, p.accent2, now);
        break;
      case buddy::Expression::Doubt:
        drawAlertMark(spi, p.accent, p.accent2, now);
        break;
      case buddy::Expression::Sad:
        drawRain(spi, p.glow, now);
        drawTear(spi, p.accent, now);
        break;
      case buddy::Expression::Angry:
        drawWarningFlash(spi, p.accent, now);
        drawAngerMark(spi, p.accent, now);
        break;
      default:
        break;
    }
  }
```

- [ ] **Step 3: Add mood backdrop helpers**

Add these private helpers before `drawHeart()`:

```cpp
  static void drawMoodBackdrop(M5Canvas* spi, const buddy_render::BuddyPalette& p,
                               buddy::Expression expr, uint32_t now,
                               uint8_t intensity) {
    uint8_t pulse = buddy_render::triWave8(now, 2200);
    uint16_t glow = buddy_render::blend565(p.glow, p.backgroundAlt, pulse);
    spi->fillRoundRect(34, 22, 252, 174, 34, glow);
    spi->fillRoundRect(54, 42, 212, 134, 28, p.face);

    switch (expr) {
      case buddy::Expression::Neutral:
        drawIdleSparkles(spi, p.accent, now);
        break;
      case buddy::Expression::Happy:
        drawBusySweep(spi, p.backgroundAlt, now, intensity);
        break;
      case buddy::Expression::Doubt:
        drawAlertAura(spi, p.accent, now);
        break;
      case buddy::Expression::Dizzy:
        drawDizzyWaves(spi, p.backgroundAlt, now);
        break;
      case buddy::Expression::Love:
        drawLoveGlow(spi, p.glow, now);
        break;
      case buddy::Expression::Angry:
        drawErrorSlashes(spi, p.backgroundAlt, now);
        break;
      default:
        break;
    }
  }

  static void drawIdleSparkles(M5Canvas* spi, uint16_t color, uint32_t now) {
    if ((now / 700) % 4 != 0) return;
    spi->fillCircle(68, 52, 3, color);
    spi->fillCircle(247, 62, 2, color);
  }

  static void drawBusySweep(M5Canvas* spi, uint16_t color, uint32_t now,
                            uint8_t intensity) {
    int16_t x = -80 + (int16_t)((now % 1200u) * 400u / 1200u);
    int count = buddy_render::sweatCountForIntensity(intensity);
    for (int i = 0; i < count; i++) {
      spi->fillRoundRect(x - i * 34, 34 + i * 18, 76, 8, 4, color);
    }
  }

  static void drawAlertAura(M5Canvas* spi, uint16_t color, uint32_t now) {
    if ((now / 180) % 2 == 0) {
      spi->drawRoundRect(26, 14, 268, 190, 36, color);
      spi->drawRoundRect(30, 18, 260, 182, 34, color);
    }
  }

  static void drawDizzyWaves(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t offset = (int16_t)((now % 900u) / 90u);
    for (int y = 30; y < 190; y += 34) {
      spi->drawLine(38 + offset, y, 118 + offset, y + 10, color);
      spi->drawLine(202 - offset, y + 8, 286 - offset, y - 2, color);
    }
  }

  static void drawLoveGlow(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t r = 18 + (int16_t)(6.0f * fabsf(sinf(now / 180.0f)));
    spi->drawCircle(160, 94, r + 76, color);
    spi->drawCircle(160, 94, r + 92, color);
  }

  static void drawErrorSlashes(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t shift = ((now / 80) % 2) ? 8 : -8;
    for (int x = 18; x < 310; x += 54) {
      spi->fillTriangle(x + shift, 24, x + 22 + shift, 24,
                        x - 20 + shift, 202, color);
    }
  }
```

- [ ] **Step 4: Update multi-color symbol function signatures**

Change the calls and definitions so the functions below accept two colors where specified:

```cpp
static void drawFloatingHearts(M5Canvas* spi, uint16_t colorA, uint16_t colorB, uint32_t now);
static void drawDizzyStars(M5Canvas* spi, uint16_t colorA, uint16_t colorB, uint32_t now);
static void drawAlertMark(M5Canvas* spi, uint16_t color, uint16_t shine, uint32_t now);
```

Inside `drawFloatingHearts`, draw the first heart with `colorA` and the second with `colorB`.

Inside `drawDizzyStars`, alternate `colorA` and `colorB`:

```cpp
uint16_t c = (i % 2 == 0) ? colorA : colorB;
drawStar(spi, sx, sy, r, c);
```

Inside `drawAlertMark`, draw the vertical bar and dot with `color`, then add:

```cpp
spi->fillRect(cx - 2, y + 4, 4, barH - 8, shine);
```

- [ ] **Step 5: Add rain and warning helpers**

Add these helpers near the existing tear and anger helpers:

```cpp
  static void drawRain(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t phase = (int16_t)((now % 900u) / 9u);
    for (int i = 0; i < 7; i++) {
      int16_t x = 42 + i * 42;
      int16_t y = (phase + i * 23) % 170;
      spi->drawLine(x, y + 26, x - 8, y + 48, color);
    }
  }

  static void drawWarningFlash(M5Canvas* spi, uint16_t color, uint32_t now) {
    if ((now / 120) % 2 == 0) {
      spi->drawTriangle(38, 34, 58, 34, 48, 54, color);
      spi->drawTriangle(268, 36, 292, 36, 280, 58, color);
    }
  }
```

- [ ] **Step 6: Compile firmware**

Run:

```bash
pio run -e m5stack-core2
```

Expected: build succeeds.

- [ ] **Step 7: Commit mood backdrop and overlays**

Run:

```bash
git add firmware/src/buddy_effect.h
git commit -m "feat(firmware): draw colorful buddy mood effects"
```

Expected: commit succeeds.

## Task 4: Recolor Custom Eyes

**Files:**
- Modify: `firmware/src/spiral_eye.h`
- Modify: `firmware/src/heart_eye.h`
- Modify: `firmware/src/alert_eye.h`
- Modify: `firmware/src/busy_eye.h`
- Modify: `firmware/src/sad_eye.h`
- Modify: `firmware/src/angry_eye.h`

- [ ] **Step 1: Add palette include to each custom eye header**

In each listed eye header, add:

```cpp
#include "buddy_palette.h"
```

- [ ] **Step 2: Recolor `SpiralEye`**

In `firmware/src/spiral_eye.h`, replace the current `color` assignment with:

```cpp
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Dizzy);
    uint16_t color = p.accent;
    uint16_t color2 = p.accent2;
```

Then replace the circle draw inside the loop with:

```cpp
      spi->fillCircle(px, py, dotR, (i % 2 == 0) ? color : color2);
```

- [ ] **Step 3: Recolor `HeartEye`**

In `firmware/src/heart_eye.h`, replace the current `color` assignment with:

```cpp
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Love);
    uint16_t color = p.accent;
```

- [ ] **Step 4: Recolor `AlertEye`**

In `firmware/src/alert_eye.h`, replace `color` and `bg` assignments with:

```cpp
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Doubt);
    uint16_t color = p.primary;
    uint16_t bg = p.face;
    uint16_t pupil = p.accent;
```

Then draw the pupil using `pupil`:

```cpp
    spi->fillCircle(cx, cy, pupilR, pupil);
```

- [ ] **Step 5: Recolor `BusyEye`**

In `firmware/src/busy_eye.h`, replace `color` and `bg` assignments with:

```cpp
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Happy);
    uint16_t color = p.primary;
    uint16_t bg = p.face;
    uint16_t pupil = p.accent;
```

Then draw the moving pupil using `pupil`:

```cpp
    spi->fillCircle(cx + dartX, cy, 5, pupil);
```

- [ ] **Step 6: Recolor `SadEye`**

In `firmware/src/sad_eye.h`, replace `color` and `bg` assignments with:

```cpp
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Sad);
    uint16_t color = p.primary;
    uint16_t bg = p.face;
    uint16_t pupil = p.secondary;
```

Then draw the low pupil using `pupil`:

```cpp
    spi->fillCircle(cx, cy + 5, 6, pupil);
```

- [ ] **Step 7: Recolor `AngryEye`**

In `firmware/src/angry_eye.h`, replace `color` and `bg` assignments with:

```cpp
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Angry);
    uint16_t color = p.primary;
    uint16_t bg = p.face;
    uint16_t pupil = p.secondary;
```

Then draw the centered pupil using `pupil`:

```cpp
    spi->fillCircle(cx, cy, 6, pupil);
```

- [ ] **Step 8: Compile firmware**

Run:

```bash
pio run -e m5stack-core2
```

Expected: build succeeds.

- [ ] **Step 9: Commit recolored eyes**

Run:

```bash
git add firmware/src/spiral_eye.h firmware/src/heart_eye.h firmware/src/alert_eye.h firmware/src/busy_eye.h firmware/src/sad_eye.h firmware/src/angry_eye.h
git commit -m "feat(firmware): recolor custom expression eyes"
```

Expected: commit succeeds.

## Task 5: Tune Per-Expression Motion

**Files:**
- Modify: `firmware/src/face_drive.h`

- [ ] **Step 1: Tune `Love` bounce**

In `firmware/src/face_drive.h`, replace the `Love` case with:

```cpp
    case buddy::Expression::Love: {
      avatar.setRotation(0.0f);
      float bounce = 1.0f + 0.10f * fabsf(sinf(now / 115.0f));
      avatar.setScale(bounce);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.62f);
      avatar.setBreath(0.0f);
      break;
    }
```

- [ ] **Step 2: Tune `Happy` busy motion**

Replace the `Happy` case with:

```cpp
    case buddy::Expression::Happy: {
      avatar.setRotation(0.025f * sinf(now / 75.0f));
      avatar.setScale(1.0f + 0.015f * fabsf(sinf(now / 210.0f)));
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.24f);
      avatar.setBreath(fabsf(sinf(now / 190.0f)));
      break;
    }
```

- [ ] **Step 3: Tune `Dizzy` rocking**

Replace the `Dizzy` case with:

```cpp
    case buddy::Expression::Dizzy: {
      avatar.setScale(1.0f);
      avatar.setRotation(0.18f * sinf(now / 135.0f));
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.38f);
      avatar.setBreath(0.0f);
      break;
    }
```

- [ ] **Step 4: Tune `Doubt`, `Sad`, and `Angry`**

Replace these three cases:

```cpp
    case buddy::Expression::Doubt: {
      avatar.setRotation(0.075f + 0.018f * sinf(now / 220.0f));
      avatar.setScale(1.0f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Sad: {
      avatar.setRotation(0.055f * sinf(now / 420.0f));
      avatar.setScale(0.98f);
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.0f);
      avatar.setBreath(0.0f);
      break;
    }
    case buddy::Expression::Angry: {
      avatar.setScale(1.0f + 0.025f * fabsf(sinf(now / 65.0f)));
      avatar.setRotation(0.065f * sinf(now / 38.0f));
      avatar.setEyeOpenRatio(1.0f);
      avatar.setMouthOpenRatio(0.48f);
      avatar.setBreath(0.0f);
      break;
    }
```

- [ ] **Step 5: Compile firmware**

Run:

```bash
pio run -e m5stack-core2
```

Expected: build succeeds.

- [ ] **Step 6: Commit motion tuning**

Run:

```bash
git add firmware/src/face_drive.h
git commit -m "feat(firmware): tune colorful expression motion"
```

Expected: commit succeeds.

## Task 6: Full Verification

**Files:**
- Read: `docs/superpowers/specs/2026-06-04-expression-animation-redesign-design.md`
- Verify: firmware tests and build output.

- [ ] **Step 1: Run all native firmware tests**

Run:

```bash
pio test -e native
```

Expected: PASS for `test_pet_state`, `test_vibration`, and `test_buddy_render`.

- [ ] **Step 2: Build Core2 firmware**

Run:

```bash
pio run -e m5stack-core2
```

Expected: build succeeds without warnings introduced by the touched files.

- [ ] **Step 3: Inspect final diff**

Run:

```bash
git status --short
git diff --stat HEAD~4..HEAD
```

Expected: clean worktree after commits, and changes limited to render palette, render drawables, `main.cpp`, tests, and plan/spec docs.

- [ ] **Step 4: On-device verification checklist**

Flash and monitor on a connected Core2:

```bash
pio run -e m5stack-core2 -t upload -t monitor
```

Expected visual checks:

- `zzz`: indigo background, closed eyes, drifting Z symbols.
- `idle`: pale teal/off-white background, friendly blink, quiet glow.
- `busy`: green/cyan mood, scanning pupils, sweat count changes with running count.
- `approve?`: coral alert mood, wide eyes, blinking exclamation mark.
- `dizzy`: violet mood, rotating spiral eyes, orbiting stars.
- `approved`: pink mood, heart eyes, floating hearts.
- `denied`: rainy blue mood, droopy eyes, falling tear.
- `error`: red/orange mood, fierce eyes, warning flash, anger mark.

- [ ] **Step 5: Commit verification notes**

If on-device verification is available, append a dated note to `firmware/M4-notes.md`:

```markdown

## M4.2 Color expression redesign verification
- `pio test -e native` passed.
- `pio run -e m5stack-core2` passed.
- On-device Core2 check: all 8 expressions were distinguishable with colored backgrounds and state-specific effects.
```

Then commit:

```bash
git add firmware/M4-notes.md
git commit -m "docs(firmware): record color expression verification"
```

Expected: commit succeeds when hardware verification was performed. If hardware is not attached, leave `firmware/M4-notes.md` unchanged and report that on-device verification remains pending.

## Self-Review

- Spec coverage: Tasks 1-2 cover palette and render integration. Task 3 covers colorful backgrounds, glows, and symbols. Task 4 covers custom eye colors. Task 5 covers motion tuning. Task 6 covers native tests, firmware build, and on-device visual acceptance criteria.
- Placeholder scan: This plan contains no unresolved placeholder sections or undefined component names.
- Type consistency: `BuddyPalette`, `paletteFor`, `rgb565`, `blend565`, `triWave8`, and `sweatCountForIntensity` are defined in Task 1 before later tasks use them.
