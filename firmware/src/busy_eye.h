#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"
#include "concept_eye_geometry.h"

// BusyEye -- Happy(busy working). A focused eye whose small pupil darts
// left-right quickly (scanning, hard at work).
class BusyEye : public m5avatar::Drawable {
 public:
  BusyEye() = default;
  ~BusyEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Happy);
    uint16_t color = p.primary;
    uint16_t sclera = p.balloonBg;
    uint16_t pupil = p.secondary;
    buddy_render::ConceptExpressionLayout l =
        buddy_render::conceptLayoutFor(buddy::Expression::Happy);
    int16_t cx = buddy_face::conceptEyeX(rect);
    int16_t cy = buddy_face::conceptEyeY();

    int16_t eyeR = l.eyeRadius;
    spi->fillEllipse(cx, cy, eyeR, eyeR + 2, color);
    spi->fillEllipse(cx, cy, eyeR - 4, eyeR - 1, sclera);
    // Small pupil darts left-right quickly (busy scanning).
    int16_t dartX = (int16_t)(10.0f * sinf(g_buddyFx.nowMs / 110.0f));
    spi->fillEllipse(cx + dartX, cy + 2, 7, 9, pupil);
    spi->fillCircle(cx - 8, cy - 10, 4, sclera);
  }
};
#endif
