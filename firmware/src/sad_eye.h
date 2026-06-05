#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"
#include "concept_eye_geometry.h"

// SadEye -- Sad(deny). Downcast half-closed eye (upper lid lowered) + worried
// brow (inner-high / outer-low). Pairs with the falling tear symbol.
class SadEye : public m5avatar::Drawable {
 public:
  explicit SadEye(bool isLeft) : isLeft_(isLeft) {}
  ~SadEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Sad);
    uint16_t color = p.primary;
    uint16_t sclera = p.balloonBg;
    uint16_t pupil = p.secondary;
    buddy_render::ConceptExpressionLayout l =
        buddy_render::conceptLayoutFor(buddy::Expression::Sad);
    int16_t cx = buddy_face::conceptEyeX(isLeft_);
    int16_t cy = buddy_face::conceptEyeY();

    int16_t eyeR = l.eyeRadius;
    spi->fillEllipse(cx, cy, eyeR, eyeR + 3, color);
    spi->fillEllipse(cx, cy, eyeR - 4, eyeR - 1, sclera);
    spi->fillEllipse(cx, cy + 8, 8, 10, pupil);
    // Lower the upper lid: erase the top portion -> half-closed/droopy.
    spi->fillRect(cx - eyeR, cy - eyeR - 3, eyeR * 2,
                  (int16_t)(eyeR * 1.05f), p.face);
    // Worried brow: inner-high / outer-low (mirrored by isLeft).
    constexpr int16_t browLen = 40;
    constexpr int16_t browY = 32;
    constexpr int16_t browDrop = 12;
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
