#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"

// BusyEye -- Happy(busy working). A focused eye whose small pupil darts
// left-right quickly (scanning, hard at work).
class BusyEye : public m5avatar::Drawable {
 public:
  BusyEye() = default;
  ~BusyEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    uint16_t color = ctx->getColorDepth() == 1
                         ? 1
                         : ctx->getColorPalette()->get(COLOR_PRIMARY);
    uint16_t bg = ctx->getColorDepth() == 1
                      ? 0
                      : ctx->getColorPalette()->get(COLOR_BACKGROUND);
    int16_t cx = rect.getCenterX();
    int16_t cy = rect.getCenterY();

    constexpr int16_t eyeR = 15;
    spi->fillCircle(cx, cy, eyeR, color);
    spi->fillCircle(cx, cy, eyeR - 3, bg);
    // Small pupil darts left-right quickly (busy scanning).
    int16_t dartX = (int16_t)(7.0f * sinf(g_buddyFx.nowMs / 110.0f));
    spi->fillCircle(cx + dartX, cy, 5, color);
  }
};
#endif
