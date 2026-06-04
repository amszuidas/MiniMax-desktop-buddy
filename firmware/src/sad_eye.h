#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"

// SadEye -- Sad(deny). Downcast half-closed eye (upper lid lowered) + worried
// brow (inner-high / outer-low). Pairs with the falling tear symbol.
class SadEye : public m5avatar::Drawable {
 public:
  explicit SadEye(bool isLeft) : isLeft_(isLeft) {}
  ~SadEye() override = default;

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

    constexpr int16_t eyeR = 16;
    spi->fillCircle(cx, cy, eyeR, color);
    spi->fillCircle(cx, cy, eyeR - 3, bg);
    spi->fillCircle(cx, cy + 5, 6, color);  // pupil low (downcast)
    // Lower the upper lid: erase the top portion -> half-closed/droopy.
    spi->fillRect(cx - eyeR, cy - eyeR - 2, eyeR * 2, (int16_t)(eyeR * 1.1f), bg);
    // Worried brow: inner-high / outer-low (mirrored by isLeft).
    constexpr int16_t browLen = 26;
    constexpr int16_t browY = 20;
    constexpr int16_t browDrop = 8;
    int16_t inner = isLeft_ ? cx + browLen / 2 : cx - browLen / 2;
    int16_t outer = isLeft_ ? cx - browLen / 2 : cx + browLen / 2;
    int16_t innerY = cy - browY;
    int16_t outerY = cy - browY + browDrop;
    spi->fillTriangle(inner, innerY, outer, outerY, inner, innerY + 4, color);
    spi->fillTriangle(outer, outerY, inner, innerY + 4, outer, outerY + 4, color);
  }

 private:
  bool isLeft_;
};
#endif
