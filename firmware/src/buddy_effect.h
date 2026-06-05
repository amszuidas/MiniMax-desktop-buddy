#pragma once
#ifndef UNIT_TEST
#include <cmath>
#include <Drawable.h>
#include "buddy_fx.h"
#include "buddy_layout.h"
#include "buddy_palette.h"

// BuddyEffect draws the approved keyframe-style scene in the Face mouth slot.
// The avatar library then draws the custom eyes above it, so this layer owns
// the full-screen mood background, face panel, mouth, and floating symbols.
class BuddyEffect : public m5avatar::Drawable {
 public:
  BuddyEffect() = default;
  ~BuddyEffect() override = default;

  void draw(M5Canvas* spi, m5avatar::BoundingRect rect,
            m5avatar::DrawContext* ctx) override {
    (void)rect;
    (void)ctx;
    const buddy::Expression expr = g_buddyFx.expr;
    const buddy_render::BuddyPalette p = buddy_render::paletteFor(expr);
    const buddy_render::ConceptExpressionLayout layout =
        buddy_render::conceptLayoutFor(expr);
    const uint32_t now = g_buddyFx.nowMs;

    drawMoodBackdrop(spi, p, layout, expr, now, g_buddyFx.intensity);
    drawConceptMouth(spi, p, layout, expr, now);
    drawForegroundSymbols(spi, p, expr, now, g_buddyFx.intensity);
  }

 private:
  static constexpr float kPi = 3.14159265358979323846f;

  static void drawThickLine(M5Canvas* spi, int16_t x0, int16_t y0,
                            int16_t x1, int16_t y1, int16_t thickness,
                            uint16_t color) {
    int16_t half = thickness / 2;
    for (int16_t d = -half; d <= half; d++) {
      spi->drawLine(x0, y0 + d, x1, y1 + d, color);
    }
    spi->fillCircle(x0, y0, half, color);
    spi->fillCircle(x1, y1, half, color);
  }

  static void drawSmile(M5Canvas* spi, int16_t cx, int16_t cy, int16_t w,
                        int16_t h, uint16_t color) {
    drawThickLine(spi, cx - w / 2, cy, cx - w / 4, cy + h / 2, 4, color);
    drawThickLine(spi, cx - w / 4, cy + h / 2, cx, cy + h, 4, color);
    drawThickLine(spi, cx, cy + h, cx + w / 4, cy + h / 2, 4, color);
    drawThickLine(spi, cx + w / 4, cy + h / 2, cx + w / 2, cy, 4, color);
  }

  static void drawFrown(M5Canvas* spi, int16_t cx, int16_t cy, int16_t w,
                        int16_t h, uint16_t color) {
    drawThickLine(spi, cx - w / 2, cy + h, cx - w / 4, cy + h / 2, 4, color);
    drawThickLine(spi, cx - w / 4, cy + h / 2, cx, cy, 4, color);
    drawThickLine(spi, cx, cy, cx + w / 4, cy + h / 2, 4, color);
    drawThickLine(spi, cx + w / 4, cy + h / 2, cx + w / 2, cy + h, 4, color);
  }

  static void drawSparkle(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
                          uint16_t color) {
    spi->drawLine(cx - r, cy, cx + r, cy, color);
    spi->drawLine(cx, cy - r, cx, cy + r, color);
    spi->fillCircle(cx, cy, 2, color);
  }

  static void drawBackgroundBubbles(M5Canvas* spi, uint16_t color,
                                    uint32_t now) {
    const int16_t bob = (int16_t)(4.0f * sinf(now / 520.0f));
    spi->drawCircle(42, 70 + bob, 12, color);
    spi->drawCircle(282, 82 - bob, 9, color);
    spi->drawCircle(68, 184 - bob, 7, color);
    spi->drawCircle(250, 190 + bob, 11, color);
  }

  static void drawMoonAndStars(M5Canvas* spi,
                               const buddy_render::BuddyPalette& p,
                               uint32_t now) {
    const int16_t bob = (int16_t)(3.0f * sinf(now / 700.0f));
    spi->fillCircle(58, 50 + bob, 24, p.accent);
    spi->fillCircle(68, 44 + bob, 23, p.background);
    drawSparkle(spi, 28, 34, 5, p.accent2);
    drawSparkle(spi, 278, 42, 4, p.accent2);
    drawSparkle(spi, 298, 98, 3, p.accent2);
  }

  static void drawFacePanel(M5Canvas* spi,
                            const buddy_render::BuddyPalette& p,
                            const buddy_render::ConceptExpressionLayout& l,
                            uint32_t now) {
    const int16_t cy = l.faceY + l.faceH / 2 +
                       (int16_t)(2.0f * sinf(now / 850.0f));
    spi->fillEllipse(buddy_render::kConceptFaceCx, cy, l.faceW / 2,
                     l.faceH / 2, p.glow);
    spi->fillEllipse(buddy_render::kConceptFaceCx, cy, l.faceW / 2 - 7,
                     l.faceH / 2 - 7, p.face);
    spi->drawEllipse(buddy_render::kConceptFaceCx, cy, l.faceW / 2 - 2,
                     l.faceH / 2 - 2,
                     buddy_render::blend565(p.primary, p.face, 170));
  }

  static void drawMoodBackdrop(
      M5Canvas* spi, const buddy_render::BuddyPalette& p,
      const buddy_render::ConceptExpressionLayout& layout,
      buddy::Expression expr, uint32_t now, uint8_t intensity) {
    spi->fillRect(0, 0, buddy_render::kConceptScreenW,
                  buddy_render::kConceptScreenH, p.background);
    uint16_t soft = buddy_render::blend565(
        p.backgroundAlt, p.glow, buddy_render::triWave8(now, 2200) / 2);
    spi->fillCircle(14, 226, 72, p.backgroundAlt);
    spi->fillCircle(308, 10, 64, soft);

    switch (expr) {
      case buddy::Expression::Sleepy:
        drawMoonAndStars(spi, p, now);
        break;
      case buddy::Expression::Neutral:
        drawBackgroundBubbles(spi, p.accent2, now);
        drawSparkle(spi, 76, 42, 6, p.accent);
        drawSparkle(spi, 244, 54, 5, p.accent);
        break;
      case buddy::Expression::Happy:
        drawBusySweep(spi, p.backgroundAlt, now, intensity);
        break;
      case buddy::Expression::Doubt:
        drawAlertAura(spi, p.accent, now);
        break;
      case buddy::Expression::Dizzy:
        drawDizzyWaves(spi, p.backgroundAlt, now);
        break;
      case buddy::Expression::Love:
        drawLoveGlow(spi, p.glow, now);
        break;
      case buddy::Expression::Sad:
        drawRain(spi, p.glow, now);
        break;
      case buddy::Expression::Angry:
        drawErrorSlashes(spi, p.backgroundAlt, now);
        break;
    }

    drawFacePanel(spi, p, layout, now);
  }

  static void drawConceptMouth(M5Canvas* spi,
                               const buddy_render::BuddyPalette& p,
                               const buddy_render::ConceptExpressionLayout& l,
                               buddy::Expression expr, uint32_t now) {
    (void)now;
    const int16_t cx = buddy_render::kConceptFaceCx;
    const int16_t y = l.mouthY;
    const uint16_t dark = p.secondary;

    switch (expr) {
      case buddy::Expression::Sleepy:
        spi->fillEllipse(cx, y, l.mouthWidth / 2, l.mouthHeight / 2, dark);
        spi->fillEllipse(cx, y - 2, l.mouthWidth / 2 - 4,
                         l.mouthHeight / 2 - 3,
                         buddy_render::rgb565(255, 118, 134));
        break;
      case buddy::Expression::Neutral:
        drawSmile(spi, cx, y - 6, l.mouthWidth, 8, dark);
        break;
      case buddy::Expression::Happy:
        spi->fillEllipse(cx, y + 2, l.mouthWidth / 2, l.mouthHeight / 2, dark);
        spi->fillEllipse(cx, y + 8, l.mouthWidth / 2 - 8,
                         l.mouthHeight / 3, p.accent);
        break;
      case buddy::Expression::Doubt:
        spi->fillEllipse(cx, y, l.mouthWidth / 2, l.mouthHeight / 2, dark);
        break;
      case buddy::Expression::Dizzy:
        drawFrown(spi, cx, y - 6, l.mouthWidth, 10, dark);
        break;
      case buddy::Expression::Love:
        spi->fillEllipse(cx, y + 2, l.mouthWidth / 2, l.mouthHeight / 2, dark);
        spi->fillEllipse(cx, y + 8, l.mouthWidth / 2 - 7,
                         l.mouthHeight / 3, p.accent);
        break;
      case buddy::Expression::Sad:
        drawFrown(spi, cx, y - 4, l.mouthWidth, 12, dark);
        break;
      case buddy::Expression::Angry:
        spi->fillRoundRect(cx - l.mouthWidth / 2, y - l.mouthHeight / 2,
                           l.mouthWidth, l.mouthHeight, 7, dark);
        spi->fillTriangle(cx - 13, y - l.mouthHeight / 2, cx - 5, y - 2,
                          cx + 3, y - l.mouthHeight / 2, p.face);
        spi->fillTriangle(cx + 8, y - l.mouthHeight / 2, cx + 16, y - 2,
                          cx + 23, y - l.mouthHeight / 2, p.face);
        break;
    }
  }

  static void drawForegroundSymbols(M5Canvas* spi,
                                    const buddy_render::BuddyPalette& p,
                                    buddy::Expression expr, uint32_t now,
                                    uint8_t intensity) {
    switch (expr) {
      case buddy::Expression::Sleepy:
        drawZzz(spi, p.accent2, now);
        break;
      case buddy::Expression::Happy:
        drawFocusBrackets(spi, p.accent2, now);
        drawSweatDrops(spi, p.accent2, now, intensity);
        break;
      case buddy::Expression::Doubt:
        drawAlertMark(spi, p.accent2, p.accent, now);
        break;
      case buddy::Expression::Dizzy:
        drawDizzyStars(spi, p.accent, p.accent2, now);
        break;
      case buddy::Expression::Love:
        drawFloatingHearts(spi, p.accent, p.accent2, now);
        drawSparkle(spi, 70, 64, 6, p.accent2);
        drawSparkle(spi, 250, 172, 5, p.accent2);
        break;
      case buddy::Expression::Sad:
        drawTear(spi, p.accent, now);
        break;
      case buddy::Expression::Angry:
        drawWarningFlash(spi, p.accent, now);
        drawAngerMark(spi, p.accent, now);
        drawBolts(spi, p.accent2, now);
        break;
      default:
        break;
    }
  }

  static void drawBusySweep(M5Canvas* spi, uint16_t color, uint32_t now,
                            uint8_t intensity) {
    int16_t x = -80 + (int16_t)((now % 900u) * 460u / 900u);
    int count = buddy_render::sweatCountForIntensity(intensity) + 1;
    for (int i = 0; i < count; i++) {
      spi->fillRoundRect(x - i * 42, 32 + i * 22, 112, 9, 4, color);
      spi->fillRoundRect(x - i * 36 + 24, 176 - i * 17, 84, 7, 3, color);
    }
  }

  static void drawFocusBrackets(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t pulse = 2 + (int16_t)(2.0f * fabsf(sinf(now / 180.0f)));
    int16_t x0 = 46 - pulse;
    int16_t x1 = 274 + pulse;
    int16_t y0 = 62 - pulse;
    int16_t y1 = 178 + pulse;
    drawThickLine(spi, x0, y0, x0 + 24, y0, 3, color);
    drawThickLine(spi, x0, y0, x0, y0 + 24, 3, color);
    drawThickLine(spi, x1, y0, x1 - 24, y0, 3, color);
    drawThickLine(spi, x1, y0, x1, y0 + 24, 3, color);
    drawThickLine(spi, x0, y1, x0 + 24, y1, 3, color);
    drawThickLine(spi, x0, y1, x0, y1 - 24, 3, color);
    drawThickLine(spi, x1, y1, x1 - 24, y1, 3, color);
    drawThickLine(spi, x1, y1, x1, y1 - 24, 3, color);
  }

  static void drawAlertAura(M5Canvas* spi, uint16_t color, uint32_t now) {
    if ((now / 180) % 2 == 0) {
      spi->drawRoundRect(18, 16, 284, 202, 34, color);
      spi->drawRoundRect(24, 22, 272, 190, 30, color);
    }
    for (int i = 0; i < 8; i++) {
      int16_t x = 32 + i * 36;
      spi->drawLine(x, 30, x + 10, 18, color);
      spi->drawLine(x + 8, 206, x + 18, 220, color);
    }
  }

  static void drawDizzyWaves(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t offset = (int16_t)((now % 900u) / 75u);
    for (int y = 28; y < 208; y += 34) {
      spi->drawLine(22 + offset, y, 92 + offset, y + 10, color);
      spi->drawLine(108 + offset, y + 10, 178 + offset, y, color);
      spi->drawLine(206 - offset, y + 8, 294 - offset, y - 3, color);
    }
  }

  static void drawLoveGlow(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t r = 22 + (int16_t)(6.0f * fabsf(sinf(now / 180.0f)));
    spi->drawCircle(160, 108, r + 88, color);
    spi->drawCircle(160, 108, r + 108, color);
  }

  static void drawErrorSlashes(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t shift = ((now / 80) % 2) ? 8 : -8;
    for (int x = 0; x < 330; x += 52) {
      spi->fillTriangle(x + shift, 20, x + 22 + shift, 20,
                        x - 26 + shift, 220, color);
    }
  }

  static void drawHeart(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
                        uint16_t color) {
    spi->fillCircle(cx - r / 2, cy, r / 2, color);
    spi->fillCircle(cx + r / 2, cy, r / 2, color);
    float a = (sqrtf(2.0f) * r) / 4.0f;
    spi->fillTriangle(cx, cy,
                      cx - r / 2 - (int16_t)a, cy + (int16_t)a,
                      cx + r / 2 + (int16_t)a, cy + (int16_t)a,
                      color);
    spi->fillTriangle(cx, cy + r / 2 + (int16_t)(2 * a),
                      cx - r / 2 - (int16_t)a, cy + (int16_t)a,
                      cx + r / 2 + (int16_t)a, cy + (int16_t)a,
                      color);
  }

  static void drawFloatingHearts(M5Canvas* spi, uint16_t colorA,
                                 uint16_t colorB, uint32_t now) {
    constexpr uint32_t period = 1900;
    float phase = (now % period) / (float)period;
    for (int i = 0; i < 5; i++) {
      float p = fmodf(phase + i * 0.2f, 1.0f);
      int16_t x = (i % 2 == 0) ? 48 + i * 18 : 246 + i * 8;
      int16_t y = 188 - (int16_t)(p * 150.0f);
      int16_t r = 10 + (int16_t)(5.0f * (1.0f - p));
      drawHeart(spi, x, y, r, (i % 2 == 0) ? colorA : colorB);
    }
  }

  static void drawSweatDrop(M5Canvas* spi, int16_t x, int16_t y, int16_t r,
                            uint16_t color) {
    spi->fillCircle(x, y, r, color);
    uint16_t a = (uint16_t)(sqrtf(3.0f) * r / 2.0f);
    spi->fillTriangle(x, y - r * 2, x - a, y - r / 2, x + a, y - r / 2,
                      color);
  }

  static void drawSweatDrops(M5Canvas* spi, uint16_t color, uint32_t now,
                             uint8_t intensity) {
    int count = buddy_render::sweatCountForIntensity(intensity);
    float phase = (now % 1000u) / 1000.0f;
    int16_t yOff = (int16_t)(phase * 18.0f);
    constexpr int16_t baseX[] = {266, 284, 248};
    constexpr int16_t baseY[] = {86, 108, 118};
    constexpr int16_t baseR[] = {9, 7, 6};
    for (int i = 0; i < count; i++) {
      drawSweatDrop(spi, baseX[i], baseY[i] + yOff, baseR[i], color);
    }
  }

  static void drawZzz(M5Canvas* spi, uint16_t color, uint32_t now) {
    auto drawZ = [&](int16_t x, int16_t y, int16_t size) {
      spi->fillRoundRect(x, y, size, 4, 2, color);
      for (int i = 0; i < size; i += 3) {
        spi->fillRect(x + size - i - 4, y + 2 + i, 5, 4, color);
      }
      spi->fillRoundRect(x, y + size, size, 4, 2, color);
    };
    float phase = (now % 2800u) / 2800.0f;
    for (int i = 0; i < 3; i++) {
      float p = fmodf(phase + i * 0.33f, 1.0f);
      if (p > 0.86f) continue;
      drawZ(222 + i * 23 + (int16_t)(p * 18.0f),
            32 - (int16_t)(p * 22.0f) + i * 16, 14 + i * 5);
    }
  }

  static void drawStar(M5Canvas* spi, int16_t cx, int16_t cy, int16_t r,
                       uint16_t color) {
    spi->fillCircle(cx, cy, r / 2, color);
    spi->fillTriangle(cx, cy - r, cx - r / 3, cy, cx + r / 3, cy, color);
    spi->fillTriangle(cx, cy + r, cx - r / 3, cy, cx + r / 3, cy, color);
    spi->fillTriangle(cx - r, cy, cx, cy - r / 3, cx, cy + r / 3, color);
    spi->fillTriangle(cx + r, cy, cx, cy - r / 3, cx, cy + r / 3, color);
  }

  static void drawDizzyStars(M5Canvas* spi, uint16_t colorA, uint16_t colorB,
                             uint32_t now) {
    float phase = (now % 1300u) / 1300.0f * 2.0f * kPi;
    constexpr int16_t orbitCx = 160;
    constexpr int16_t orbitCy = 58;
    constexpr int16_t orbitR = 88;
    for (int i = 0; i < 4; i++) {
      float angle = phase + i * (2.0f * kPi / 4.0f);
      int16_t sx = orbitCx + (int16_t)(orbitR * cosf(angle));
      int16_t sy = orbitCy + (int16_t)((orbitR * 0.42f) * sinf(angle));
      drawStar(spi, sx, sy, 9 + (i % 2), (i % 2 == 0) ? colorA : colorB);
    }
  }

  static void drawAlertMark(M5Canvas* spi, uint16_t color, uint16_t shine,
                            uint32_t now) {
    if ((now % 700u) > 540u) return;
    int16_t bob = (int16_t)(5.0f * sinf(now / 130.0f));
    constexpr int16_t cx = buddy_render::kAlertMarkX;
    int16_t y = buddy_render::kAlertMarkY + bob;
    spi->fillRoundRect(cx - 8, y, 16, 46, 6, color);
    spi->fillRect(cx - 2, y + 7, 4, 30, shine);
    spi->fillCircle(cx, y + 62, 10, color);
  }

  static void drawRain(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t phase = (int16_t)((now % 900u) / 7u);
    for (int i = 0; i < 8; i++) {
      int16_t x = 28 + i * 39;
      int16_t y = (phase + i * 27) % 190;
      drawThickLine(spi, x, y, x - 10, y + 30, 2, color);
    }
  }

  static void drawTear(M5Canvas* spi, uint16_t color, uint32_t now) {
    float phase = (now % 1600u) / 1600.0f;
    int16_t y = 122 + (int16_t)(phase * 54.0f);
    drawSweatDrop(spi, buddy_render::kSadTearX, y, 10, color);
  }

  static void drawWarningFlash(M5Canvas* spi, uint16_t color, uint32_t now) {
    if ((now / 120) % 2 != 0) return;
    spi->drawTriangle(32, 34, 60, 34, 46, 62, color);
    spi->drawTriangle(260, 36, 294, 36, 277, 68, color);
    spi->drawLine(46, 42, 46, 53, color);
    spi->drawLine(277, 44, 277, 57, color);
  }

  static void drawAngerMark(M5Canvas* spi, uint16_t color, uint32_t now) {
    constexpr int16_t cx = 274;
    constexpr int16_t cy = 58;
    int16_t r = 15 + (int16_t)(3.0f * fabsf(sinf(now / 120.0f)));
    spi->fillRoundRect(cx - r, cy - r, r + 4, 5, 2, color);
    spi->fillRoundRect(cx - r, cy - r, 5, r + 4, 2, color);
    spi->fillRoundRect(cx, cy, r + 4, 5, 2, color);
    spi->fillRoundRect(cx + r - 1, cy, 5, r + 4, 2, color);
  }

  static void drawBolts(M5Canvas* spi, uint16_t color, uint32_t now) {
    int16_t shift = ((now / 90) % 2) ? 3 : -3;
    spi->fillTriangle(58 + shift, 104, 84 + shift, 104, 68 + shift, 138,
                      color);
    spi->fillTriangle(68 + shift, 128, 96 + shift, 128, 54 + shift, 176,
                      color);
    spi->fillTriangle(246 - shift, 106, 272 - shift, 106, 256 - shift, 140,
                      color);
    spi->fillTriangle(256 - shift, 130, 286 - shift, 130, 240 - shift, 178,
                      color);
  }
};
#endif
