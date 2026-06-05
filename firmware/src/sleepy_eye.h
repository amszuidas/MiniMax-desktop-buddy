#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"
#include "concept_eye_geometry.h"

// SleepyEye -- Sleepy/disconnected. Closed crescent eyes from the keyframe.
class SleepyEye : public m5avatar::Drawable {
 public:
  SleepyEye() = default;
  ~SleepyEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    const buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Sleepy);
    const buddy_render::ConceptExpressionLayout l =
        buddy_render::conceptLayoutFor(buddy::Expression::Sleepy);
    const int16_t cx = buddy_face::conceptEyeX(rect);
    const int16_t cy = buddy_face::conceptEyeY() + 4;
    const int16_t r = l.eyeRadius;
    const int16_t bob = (int16_t)(2.0f * sinf(g_buddyFx.nowMs / 900.0f));

    drawCrescent(spi, cx, cy + bob, r, p.primary);
    spi->fillCircle(cx + r - 4, cy + 7 + bob, 3, p.accent2);
  }

 private:
  static void drawCrescent(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
                           uint16_t color) {
    drawSegment(spi, cx - r, cy - 1, cx - r / 2, cy + 6, color);
    drawSegment(spi, cx - r / 2, cy + 6, cx, cy + 9, color);
    drawSegment(spi, cx, cy + 9, cx + r / 2, cy + 6, color);
    drawSegment(spi, cx + r / 2, cy + 6, cx + r, cy - 1, color);
  }

  static void drawSegment(M5Canvas* spi, int16_t x0, int16_t y0, int16_t x1,
                          int16_t y1, uint16_t color) {
    for (int16_t d = -2; d <= 2; d++) {
      spi->drawLine(x0, y0 + d, x1, y1 + d, color);
    }
  }
};
#endif
