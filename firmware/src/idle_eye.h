#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"
#include "concept_eye_geometry.h"

// IdleEye -- Neutral. Large glossy eyes from the approved keyframe sheet.
class IdleEye : public m5avatar::Drawable {
 public:
  IdleEye() = default;
  ~IdleEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    const buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Neutral);
    const buddy_render::ConceptExpressionLayout l =
        buddy_render::conceptLayoutFor(buddy::Expression::Neutral);
    const int16_t cx = buddy_face::conceptEyeX(rect);
    const int16_t cy = buddy_face::conceptEyeY();
    const int16_t r = l.eyeRadius;

    uint32_t blinkPhase = g_buddyFx.nowMs % 4200u;
    if (blinkPhase > 3650u && blinkPhase < 3800u) {
      spi->fillRoundRect(cx - r, cy - 2, r * 2, 5, 2, p.primary);
      return;
    }

    int16_t bob = (int16_t)(1.5f * sinf(g_buddyFx.nowMs / 620.0f));
    spi->fillEllipse(cx, cy + bob, r, r + 2, p.primary);
    spi->fillEllipse(cx, cy + bob, r - 4, r - 2, p.balloonBg);
    spi->fillEllipse(cx + 5, cy + 5 + bob, 9, 11, p.secondary);
    spi->fillCircle(cx - 8, cy - 9 + bob, 5, p.balloonBg);
    spi->fillCircle(cx - 2, cy - 14 + bob, 3, p.accent2);
  }
};
#endif
