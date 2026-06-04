#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include <Mouth.h>
#include "buddy_fx.h"
#include "buddy_palette.h"

// BuddyEffect -- a composite Drawable that:
//   1) Delegates to the real Mouth so the mouth is never lost.
//   2) Overlays per-expression symbols (hearts, sweat drops, Zzz, stars)
//      on top, using absolute sprite coordinates (the spi canvas is the
//      full 320x240 face sprite).
//
// Installed via Face::setMouth() to piggyback on the mouth drawing slot.
// This is the cleanest integration path: the library's Face has no
// setEffect() and addTask() provides no canvas access.
class BuddyEffect : public m5avatar::Drawable {
 public:
  // Takes ownership of nothing; caller manages mouthDelegate lifetime.
  explicit BuddyEffect(m5avatar::Drawable* mouthDelegate)
      : mouth_(mouthDelegate) {}
  ~BuddyEffect() override = default;

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

 private:
  m5avatar::Drawable* mouth_;

  // -- Heart helpers (simplified from Effect.h drawHeartMark) ---------------

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
      spi->fillTriangle(x + shift, 24, x + 22 + shift,
                        24, x - 20 + shift, 202, color);
    }
  }

  static void drawHeart(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
                        uint16_t color) {
    spi->fillCircle(cx - r / 2, cy, r / 2, color);
    spi->fillCircle(cx + r / 2, cy, r / 2, color);
    float a = (sqrtf(2.0f) * r) / 4.0f;
    spi->fillTriangle(cx, cy,
                      cx - r / 2 - (int16_t)a, cy + (int16_t)a,
                      cx + r / 2 + (int16_t)a, cy + (int16_t)a,
                      color);
    spi->fillTriangle(cx, cy + r / 2 + (int16_t)(2 * a),
                      cx - r / 2 - (int16_t)a, cy + (int16_t)a,
                      cx + r / 2 + (int16_t)a, cy + (int16_t)a,
                      color);
  }

  // Two hearts float upward in a 2-second cycle.
  static void drawFloatingHearts(M5Canvas* spi, uint16_t colorA,
                                 uint16_t colorB, uint32_t now) {
    constexpr uint32_t period = 2000;
    float phase = (now % period) / (float)period;  // 0..1

    // Heart 1: left side, offset by 0
    {
      int16_t x = 270;
      int16_t yBase = 110;
      int16_t yTop = 20;
      int16_t y = yBase - (int16_t)((yBase - yTop) * phase);
      int16_t r = 9 + (int16_t)(4.0f * (1.0f - phase));  // shrinks as rises
      drawHeart(spi, x, y, r, colorA);
    }
    // Heart 2: right side, 180-degree phase offset
    {
      float phase2 = fmodf(phase + 0.5f, 1.0f);
      int16_t x = 290;
      int16_t yBase = 120;
      int16_t yTop = 25;
      int16_t y = yBase - (int16_t)((yBase - yTop) * phase2);
      int16_t r = 8 + (int16_t)(3.0f * (1.0f - phase2));
      drawHeart(spi, x, y, r, colorB);
    }
  }

  // -- Sweat drops (adapted from Effect.h drawSweatMark) --------------------

  static void drawSweatDrop(M5Canvas* spi, int16_t x, int16_t y, int16_t r,
                            uint16_t color) {
    spi->fillCircle(x, y, r, color);
    uint16_t a = (uint16_t)(sqrtf(3.0f) * r / 2.0f);
    spi->fillTriangle(x, y - r * 2, x - a, y - r / 2, x + a, y - r / 2,
                      color);
  }

  // 1-3 drops based on intensity, sliding down over time.
  static void drawSweatDrops(M5Canvas* spi, uint16_t color, uint32_t now,
                             uint8_t intensity) {
    int count = 1;
    if (intensity >= 240) count = 3;
    else if (intensity >= 160) count = 2;

    constexpr uint32_t period = 1200;
    float phase = (now % period) / (float)period;
    int16_t yOff = (int16_t)(phase * 20.0f);  // slide down 20px

    // Base positions near right side of face
    constexpr int16_t baseX[] = {288, 275, 300};
    constexpr int16_t baseY[] = {95, 105, 100};
    constexpr int16_t baseR[] = {8, 6, 5};

    for (int i = 0; i < count; i++) {
      drawSweatDrop(spi, baseX[i], baseY[i] + yOff, baseR[i], color);
    }
  }

  // -- Zzz (sleepy bubbles) ------------------------------------------------

  static void drawZzz(M5Canvas* spi, uint16_t color, uint32_t now) {
    constexpr uint32_t period = 3000;
    float phase = (now % period) / (float)period;

    // Three Z letters floating up-right, staggered in time.
    // Using simple fillRect to draw a blocky Z.
    auto drawZ = [&](int16_t x, int16_t y, int16_t size) {
      // Top bar
      spi->fillRect(x, y, size, 2, color);
      // Diagonal (approximated with two small rects)
      for (int i = 0; i < size; i++) {
        int16_t dx = size - 1 - (i * (size - 1)) / (size > 1 ? size - 1 : 1);
        spi->fillRect(x + dx, y + 1 + (i * (size - 3)) / (size > 1 ? size - 1 : 1), 2, 2, color);
      }
      // Bottom bar
      spi->fillRect(x, y + size - 2, size, 2, color);
    };

    // Three Zs at increasing distance from face, phase-staggered
    for (int i = 0; i < 3; i++) {
      float p = fmodf(phase + i * 0.33f, 1.0f);
      int16_t x = 270 + (int16_t)(p * 30.0f) + i * 8;
      int16_t y = 60 - (int16_t)(p * 40.0f);
      int16_t sz = 14 + i * 4;  // larger Zs further out
      // Fade by skipping draw when near cycle end
      if (p < 0.85f) {
        drawZ(x, y, sz);
      }
    }
  }

  // -- Dizzy stars ---------------------------------------------------------

  static void drawStar(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
                       uint16_t color) {
    // Simple 4-pointed star using two overlapping triangles + center circle.
    spi->fillCircle(cx, cy, r / 2, color);
    // Vertical diamond
    spi->fillTriangle(cx, cy - r, cx - r / 3, cy, cx + r / 3, cy, color);
    spi->fillTriangle(cx, cy + r, cx - r / 3, cy, cx + r / 3, cy, color);
    // Horizontal diamond
    spi->fillTriangle(cx - r, cy, cx, cy - r / 3, cx, cy + r / 3, color);
    spi->fillTriangle(cx + r, cy, cx, cy - r / 3, cx, cy + r / 3, color);
  }

  // Stars orbit around the head area.
  static void drawDizzyStars(M5Canvas* spi, uint16_t colorA, uint16_t colorB,
                             uint32_t now) {
    constexpr uint32_t period = 1400;
    float phase = (now % period) / (float)period * 2.0f * M_PI;

    // Orbit center roughly above the face center, three stars 120 degrees apart
    constexpr int16_t orbitCx = 160;
    constexpr int16_t orbitCy = 45;
    constexpr int16_t orbitR = 70;

    for (int i = 0; i < 3; i++) {
      float angle = phase + i * (2.0f * M_PI / 3.0f);
      int16_t sx = orbitCx + (int16_t)(orbitR * cosf(angle));
      int16_t sy = orbitCy + (int16_t)((orbitR * 0.4f) * sinf(angle));  // elliptical
      int16_t r = 8 + (i % 2);  // slight size variation
      uint16_t c = (i % 2 == 0) ? colorA : colorB;
      drawStar(spi, sx, sy, r, c);
    }
  }

  // -- Alert mark (Doubt: big exclamation) ---------------------------------

  // A big exclamation mark drawn at the upper-right of the face, blinking.
  static void drawAlertMark(M5Canvas* spi, uint16_t color, uint16_t shine,
                            uint32_t now) {
    // Blink: visible ~70% of a 700ms cycle.
    constexpr uint32_t period = 700;
    float phase = (now % period) / (float)period;
    if (phase > 0.7f) return;

    constexpr int16_t cx = 160;   // top-center, above the face
    constexpr int16_t topY = 8;
    constexpr int16_t barW = 12;
    constexpr int16_t barH = 34;
    // Bob up/down for a lively "hey, look!" motion.
    int16_t bob = (int16_t)(5.0f * sinf(now / 130.0f));
    int16_t y = topY + bob;
    // Vertical bar.
    spi->fillRect(cx - barW / 2, y, barW, barH, color);
    spi->fillRect(cx - 2, y + 4, 4, barH - 8, shine);
    // Dot below the bar.
    constexpr int16_t gap = 12;
    constexpr int16_t dotR = barW / 2 + 1;
    spi->fillCircle(cx, y + barH + gap, dotR, color);
  }

  // -- Tear (Sad: big teardrop sliding down) -------------------------------

  static void drawRain(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t phase = (int16_t)((now % 900u) / 9u);
    for (int i = 0; i < 7; i++) {
      int16_t x = 42 + i * 42;
      int16_t y = (phase + i * 23) % 170;
      spi->drawLine(x, y + 26, x - 8, y + 48, color);
    }
  }

  // A big teardrop sliding down from the eye corner, looping.
  static void drawTear(M5Canvas* spi, uint16_t color, uint32_t now) {
    constexpr uint32_t period = 1800;
    float phase = (now % period) / (float)period;  // 0..1

    constexpr int16_t x = 110;          // left-eye corner area
    constexpr int16_t yTop = 120;
    constexpr int16_t yBot = 200;
    int16_t y = yTop + (int16_t)((yBot - yTop) * phase);
    int16_t r = 9;                       // big drop
    // Teardrop: circle body + triangle pointing up (reuse drawSweatDrop shape).
    drawSweatDrop(spi, x, y, r, color);
  }

  // -- Anger mark (Angry: pulsing manga vein, upper-right) ------------------

  static void drawWarningFlash(M5Canvas* spi, uint16_t color, uint32_t now) {
    if ((now / 120) % 2 == 0) {
      spi->drawTriangle(38, 34, 58, 34, 48, 54, color);
      spi->drawTriangle(268, 36, 292, 36, 280, 58, color);
    }
  }

  static void drawAngerMark(M5Canvas* spi, uint16_t color, uint32_t now) {
    constexpr int16_t cx = 275;
    constexpr int16_t cy = 55;
    int16_t r = 14 + (int16_t)(3.0f * fabsf(sinf(now / 120.0f)));  // pulse
    // Two opposite right-angles -> a '#'-ish anger burst.
    spi->fillRect(cx - r, cy - r, r, 4, color);
    spi->fillRect(cx - r, cy - r, 4, r, color);
    spi->fillRect(cx, cy, r, 4, color);
    spi->fillRect(cx + r - 4, cy, 4, r, color);
  }
};
#endif
