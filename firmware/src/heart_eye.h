#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"
#include "concept_eye_geometry.h"

// HeartEye -- replaces the default Eye Drawable when Love.
// Draws a solid heart that pulses ("heartbeat") over time using g_buddyFx.nowMs.
// Heart geometry mirrors buddy_effect.h drawHeart (two fillCircle lobes + two
// fillTriangle bottom), reimplemented locally so each eye is self-contained.
class HeartEye : public m5avatar::Drawable {
 public:
  HeartEye() = default;
  ~HeartEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Love);
    buddy_render::ConceptExpressionLayout l =
        buddy_render::conceptLayoutFor(buddy::Expression::Love);
    int16_t cx = buddy_face::conceptEyeX(rect);
    int16_t cy = buddy_face::conceptEyeY();
    int16_t eyeR = l.eyeRadius;

    spi->fillEllipse(cx, cy, eyeR, eyeR + 2, p.primary);
    spi->fillEllipse(cx, cy, eyeR - 4, eyeR - 1, p.balloonBg);

    // Heartbeat: radius pulses base..base+amp. |sin| doubles the visual rate.
    // k=260 keeps it a calm, gentle beat (not frantic).
    float base = l.eyeRadius - 11.0f;
    constexpr float amp = 3.0f;
    constexpr float k = 260.0f;
    int16_t r = (int16_t)(base + amp * fabsf(sinf(g_buddyFx.nowMs / k)));

    // The heart extends further down (tip at cy+1.207r) than up (cy-0.5r), so
    // nudge the center up a touch to keep the tip inside the eye socket.
    drawSolidHeart(spi, cx, cy - 3, r, p.accent);
    spi->fillCircle(cx - 8, cy - 11, 3, p.balloonBg);
  }

 private:
  // Solid heart centered at (cx, cy) with overall radius r.
  // Same construction as buddy_effect.h drawHeart (kept local on purpose).
  static void drawSolidHeart(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
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
};
#endif
