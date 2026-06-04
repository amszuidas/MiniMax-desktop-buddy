#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"

// SpiralEye -- replaces the default Eye Drawable when Dizzy.
// Draws an Archimedean spiral that rotates over time using g_buddyFx.nowMs.
class SpiralEye : public m5avatar::Drawable {
 public:
  SpiralEye() = default;
  ~SpiralEye() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    uint16_t color = ctx->getColorDepth() == 1
                         ? 1
                         : ctx->getColorPalette()->get(COLOR_PRIMARY);

    int16_t cx = rect.getCenterX();
    int16_t cy = rect.getCenterY();

    // Rotation phase: full revolution every ~1.5 s
    float phase = (g_buddyFx.nowMs % 1500u) / 1500.0f * 2.0f * M_PI;

    // Draw an Archimedean spiral as a series of small filled circles.
    // r = a * theta, theta from 0 to 3*PI (1.5 turns).
    constexpr float maxTheta = 3.0f * M_PI;
    constexpr int steps = 30;
    constexpr float a = 2.2f;  // growth rate (pixels per radian)

    for (int i = 0; i <= steps; i++) {
      float t = maxTheta * i / steps;
      float r = a * t;
      float angle = t + phase;
      int16_t px = cx + (int16_t)(r * cosf(angle));
      int16_t py = cy + (int16_t)(r * sinf(angle));
      // dot radius tapers from 3 at center to 2 at edge
      int16_t dotR = (i < steps / 2) ? 4 : 3;
      spi->fillCircle(px, py, dotR, color);
    }
  }
};
#endif
