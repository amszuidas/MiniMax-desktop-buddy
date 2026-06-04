#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include <Mouth.h>
#include "buddy_fx.h"

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
    // Step 1: draw the real mouth so it is never lost.
    if (mouth_) {
      mouth_->draw(spi, rect, ctx);
    }

    // Step 2: overlay effect symbols based on current expression.
    uint16_t color = ctx->getColorDepth() == 1
                         ? 1
                         : ctx->getColorPalette()->get(COLOR_PRIMARY);
    uint32_t now = g_buddyFx.nowMs;

    switch (g_buddyFx.expr) {
      case buddy::Expression::Love:
        drawFloatingHearts(spi, color, now);
        break;
      case buddy::Expression::Happy:
        drawSweatDrops(spi, color, now, g_buddyFx.intensity);
        break;
      case buddy::Expression::Sleepy:
        drawZzz(spi, color, now);
        break;
      case buddy::Expression::Dizzy:
        drawDizzyStars(spi, color, now);
        break;
      case buddy::Expression::Doubt:
        drawAlertMark(spi, color, now);
        break;
      case buddy::Expression::Sad:
        drawTear(spi, color, now);
        break;
      default:
        break;
    }
  }

 private:
  m5avatar::Drawable* mouth_;

  // -- Heart helpers (simplified from Effect.h drawHeartMark) ---------------

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
  static void drawFloatingHearts(M5Canvas* spi, uint16_t color, uint32_t now) {
    constexpr uint32_t period = 2000;
    float phase = (now % period) / (float)period;  // 0..1

    // Heart 1: left side, offset by 0
    {
      int16_t x = 270;
      int16_t yBase = 110;
      int16_t yTop = 20;
      int16_t y = yBase - (int16_t)((yBase - yTop) * phase);
      int16_t r = 14 + (int16_t)(6.0f * (1.0f - phase));  // shrinks as rises
      drawHeart(spi, x, y, r, color);
    }
    // Heart 2: right side, 180-degree phase offset
    {
      float phase2 = fmodf(phase + 0.5f, 1.0f);
      int16_t x = 290;
      int16_t yBase = 120;
      int16_t yTop = 25;
      int16_t y = yBase - (int16_t)((yBase - yTop) * phase2);
      int16_t r = 12 + (int16_t)(5.0f * (1.0f - phase2));
      drawHeart(spi, x, y, r, color);
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
    constexpr int16_t baseR[] = {10, 8, 7};

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
  static void drawDizzyStars(M5Canvas* spi, uint16_t color, uint32_t now) {
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
      drawStar(spi, sx, sy, r, color);
    }
  }

  // -- Alert mark (Doubt: big exclamation) ---------------------------------

  // A big exclamation mark drawn at the upper-right of the face, blinking.
  static void drawAlertMark(M5Canvas* spi, uint16_t color, uint32_t now) {
    // Blink: visible ~70% of a 700ms cycle.
    constexpr uint32_t period = 700;
    float phase = (now % period) / (float)period;
    if (phase > 0.7f) return;

    constexpr int16_t cx = 285;   // upper-right of face
    constexpr int16_t topY = 30;
    constexpr int16_t barW = 10;
    constexpr int16_t barH = 34;
    // Vertical bar.
    spi->fillRect(cx - barW / 2, topY, barW, barH, color);
    // Dot below the bar.
    constexpr int16_t gap = 12;
    constexpr int16_t dotR = barW / 2 + 1;
    spi->fillCircle(cx, topY + barH + gap, dotR, color);
  }

  // -- Tear (Sad: big teardrop sliding down) -------------------------------

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
};
#endif
