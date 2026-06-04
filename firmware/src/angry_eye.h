#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_palette.h"

// AngryEye -- Angry(error). Staring eye + fierce brow (inner-low / outer-high)
// pressing down on the inner corner. Pairs with the anger mark + face shake.
class AngryEye : public m5avatar::Drawable {
 public:
  explicit AngryEye(bool isLeft) : isLeft_(isLeft) {}
  ~AngryEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Angry);
    uint16_t color = p.primary;
    uint16_t bg = p.face;
    uint16_t pupil = p.secondary;
    int16_t cx = rect.getCenterX();
    int16_t cy = rect.getCenterY();

    constexpr int16_t eyeR = 16;
    spi->fillCircle(cx, cy, eyeR, color);
    spi->fillCircle(cx, cy, eyeR - 3, bg);
    spi->fillCircle(cx, cy, 6, pupil);  // staring pupil centered
    // Fierce brow: inner-low / outer-high (mirrored), thick.
    constexpr int16_t browLen = 28;
    constexpr int16_t browY = 20;
    constexpr int16_t browRise = 9;
    int16_t inner = isLeft_ ? cx + browLen / 2 : cx - browLen / 2;
    int16_t outer = isLeft_ ? cx - browLen / 2 : cx + browLen / 2;
    int16_t innerY = cy - browY + browRise;  // inner low
    int16_t outerY = cy - browY;             // outer high
    spi->fillTriangle(inner, innerY, outer, outerY, inner, innerY + 5, color);
    spi->fillTriangle(outer, outerY, inner, innerY + 5, outer, outerY + 5, color);
    // Press brow onto eye: erase a wedge at the inner-top of the eye.
    // Stop the wedge above the pupil (pupil top is cy-6) so it isn't bitten.
    spi->fillTriangle(cx, cy - eyeR, inner, innerY, cx, cy - 8, bg);
  }

 private:
  bool isLeft_;
};
#endif
