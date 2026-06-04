#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"

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
    uint16_t color = ctx->getColorDepth() == 1
                         ? 1
                         : ctx->getColorPalette()->get(COLOR_PRIMARY);
    uint16_t bg = ctx->getColorDepth() == 1
                      ? 0
                      : ctx->getColorPalette()->get(COLOR_BACKGROUND);

    int16_t cx = rect.getCenterX();
    int16_t cy = rect.getCenterY();

    // Wide-open eye: a big filled circle (the eye), with a smaller background
    // circle punched out and a pulsing pupil in the middle -> "staring at you".
    constexpr int16_t eyeR = 17;
    spi->fillCircle(cx, cy, eyeR, color);          // eye outline mass
    spi->fillCircle(cx, cy, eyeR - 3, bg);         // white of the eye
    // Pupil pulses small<->slightly-bigger so it feels alert/alive.
    int16_t pupilR = 6 + (int16_t)(2.0f * fabsf(sinf(g_buddyFx.nowMs / 280.0f)));
    spi->fillCircle(cx, cy, pupilR, color);

    // Slanted brow line above the eye: inner end high, outer end low (frown).
    // Mirror by isLeft so both brows tilt toward the face center.
    constexpr int16_t browLen = 26;
    constexpr int16_t browY = 22;   // above the eye center
    constexpr int16_t browDrop = 9; // vertical slant
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
