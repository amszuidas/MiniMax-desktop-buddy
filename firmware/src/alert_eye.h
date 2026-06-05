#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"
#include "concept_eye_geometry.h"

// AlertEye -- replaces the default Eye Drawable when Doubt (approval pending).
// A wide alert eye: large open eye with a small pupil that subtly pulses, plus a
// slanted brow line above (inner-high / outer-low = a tense, watchful frown).
// Reads g_buddyFx.nowMs for the pupil pulse. isLeft mirrors the brow slant so
// both brows angle toward the center (a worried look).
class AlertEye : public m5avatar::Drawable {
 public:
  explicit AlertEye(bool isLeft) : isLeft_(isLeft) {}
  ~AlertEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)ctx;
    buddy_render::BuddyPalette p =
        buddy_render::paletteFor(buddy::Expression::Doubt);
    uint16_t color = p.primary;
    uint16_t sclera = p.balloonBg;
    uint16_t pupil = p.accent;

    buddy_render::ConceptExpressionLayout l =
        buddy_render::conceptLayoutFor(buddy::Expression::Doubt);
    int16_t cx = buddy_face::conceptEyeX(isLeft_);
    int16_t cy = buddy_face::conceptEyeY();

    int16_t eyeR = l.eyeRadius;
    spi->fillEllipse(cx, cy, eyeR, eyeR + 3, color);
    spi->fillEllipse(cx, cy, eyeR - 4, eyeR - 1, sclera);
    // Pupil pulses small<->slightly-bigger so it feels alert/alive.
    int16_t pupilR = 7 + (int16_t)(2.0f * fabsf(sinf(g_buddyFx.nowMs / 280.0f)));
    spi->fillEllipse(cx, cy + 2, pupilR, pupilR + 2, pupil);
    spi->fillCircle(cx - 8, cy - 10, 4, sclera);

    // Slanted brow line above the eye: inner end high, outer end low (frown).
    // Mirror by isLeft so both brows tilt toward the face center.
    constexpr int16_t browLen = 40;
    constexpr int16_t browY = 32;
    constexpr int16_t browDrop = 12;
    int16_t inner = isLeft_ ? cx + browLen / 2 : cx - browLen / 2;
    int16_t outer = isLeft_ ? cx - browLen / 2 : cx + browLen / 2;
    int16_t innerY = cy - browY;            // inner end high
    int16_t outerY = cy - browY + browDrop; // outer end low
    // Thick line via two offset triangles (a filled quad).
    spi->fillTriangle(inner, innerY, outer, outerY, inner, innerY + 4, color);
    spi->fillTriangle(outer, outerY, inner, innerY + 4, outer, outerY + 4, color);
  }

 private:
  bool isLeft_;
};
#endif
