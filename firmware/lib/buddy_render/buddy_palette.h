#pragma once

#include <cstdint>

#include "pet_state.h"

namespace buddy_render {

constexpr int kAvatarColorDepth = 16;

struct BuddyPalette {
  uint16_t background;
  uint16_t backgroundAlt;
  uint16_t face;
  uint16_t primary;
  uint16_t secondary;
  uint16_t accent;
  uint16_t accent2;
  uint16_t glow;
  uint16_t balloonBg;
  uint16_t balloonFg;
};

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8u) << 8) |
                               ((g & 0xFCu) << 3) |
                               (b >> 3));
}

constexpr uint8_t red8(uint16_t c) {
  return static_cast<uint8_t>((((c >> 11) & 0x1Fu) * 255u) / 31u);
}

constexpr uint8_t green8(uint16_t c) {
  return static_cast<uint8_t>((((c >> 5) & 0x3Fu) * 255u) / 63u);
}

constexpr uint8_t blue8(uint16_t c) {
  return static_cast<uint8_t>(((c & 0x1Fu) * 255u) / 31u);
}

inline uint16_t blend565(uint16_t a, uint16_t b, uint8_t t) {
  const uint16_t inv = static_cast<uint16_t>(255u - t);
  const uint8_t r = static_cast<uint8_t>((red8(a) * inv + red8(b) * t) / 255u);
  const uint8_t g = static_cast<uint8_t>((green8(a) * inv + green8(b) * t) / 255u);
  const uint8_t bl = static_cast<uint8_t>((blue8(a) * inv + blue8(b) * t) / 255u);
  return rgb565(r, g, bl);
}

inline uint8_t triWave8(uint32_t nowMs, uint16_t periodMs) {
  if (periodMs == 0) {
    return 0;
  }

  const uint32_t phase = nowMs % periodMs;
  const uint32_t half = periodMs / 2u;
  if (half == 0) {
    return 0;
  }

  if (phase <= half) {
    return static_cast<uint8_t>((phase * 255u) / half);
  }
  return static_cast<uint8_t>(255u - (((phase - half) * 255u) / half));
}

inline int sweatCountForIntensity(uint8_t intensity) {
  if (intensity >= 240) {
    return 3;
  }
  if (intensity >= 160) {
    return 2;
  }
  return 1;
}

inline BuddyPalette paletteFor(buddy::Expression e) {
  switch (e) {
    case buddy::Expression::Sleepy:
      return {
          rgb565(18, 33, 68),    rgb565(34, 52, 96),    rgb565(246, 238, 208),
          rgb565(133, 190, 255), rgb565(72, 116, 190),  rgb565(244, 215, 112),
          rgb565(159, 223, 255), rgb565(91, 141, 255),  rgb565(236, 246, 255),
          rgb565(24, 38, 72),
      };
    case buddy::Expression::Neutral:
      return {
          rgb565(26, 71, 86),    rgb565(43, 97, 111),   rgb565(250, 239, 214),
          rgb565(91, 220, 196),  rgb565(33, 154, 159),  rgb565(255, 199, 95),
          rgb565(144, 235, 218), rgb565(94, 218, 237),  rgb565(242, 251, 245),
          rgb565(25, 55, 58),
      };
    case buddy::Expression::Happy:
      return {
          rgb565(29, 78, 52),    rgb565(47, 116, 73),   rgb565(255, 244, 210),
          rgb565(111, 232, 114), rgb565(46, 174, 87),   rgb565(255, 214, 64),
          rgb565(152, 242, 161), rgb565(153, 239, 108), rgb565(246, 255, 235),
          rgb565(26, 63, 42),
      };
    case buddy::Expression::Doubt:
      return {
          rgb565(84, 49, 35),    rgb565(126, 70, 43),   rgb565(255, 236, 204),
          rgb565(255, 173, 69),  rgb565(192, 99, 45),   rgb565(255, 75, 92),
          rgb565(255, 215, 114), rgb565(255, 133, 87),  rgb565(255, 247, 227),
          rgb565(74, 45, 34),
      };
    case buddy::Expression::Dizzy:
      return {
          rgb565(43, 39, 104),   rgb565(63, 58, 145),   rgb565(249, 240, 219),
          rgb565(134, 116, 255), rgb565(73, 208, 226),  rgb565(255, 94, 190),
          rgb565(255, 219, 82),  rgb565(171, 115, 255), rgb565(248, 242, 255),
          rgb565(38, 33, 80),
      };
    case buddy::Expression::Love:
      return {
          rgb565(92, 35, 73),    rgb565(136, 55, 104),  rgb565(255, 235, 221),
          rgb565(255, 111, 169), rgb565(207, 68, 128),  rgb565(255, 205, 219),
          rgb565(255, 151, 118), rgb565(255, 117, 180), rgb565(255, 243, 248),
          rgb565(79, 35, 61),
      };
    case buddy::Expression::Sad:
      return {
          rgb565(30, 62, 105),   rgb565(44, 86, 139),   rgb565(241, 238, 223),
          rgb565(104, 163, 235), rgb565(62, 109, 185),  rgb565(126, 225, 255),
          rgb565(186, 206, 255), rgb565(81, 143, 222),  rgb565(240, 248, 255),
          rgb565(28, 52, 86),
      };
    case buddy::Expression::Angry:
      return {
          rgb565(100, 27, 37),   rgb565(150, 38, 42),   rgb565(255, 229, 201),
          rgb565(255, 82, 66),   rgb565(196, 38, 44),   rgb565(255, 190, 40),
          rgb565(255, 122, 84),  rgb565(255, 64, 48),   rgb565(255, 241, 222),
          rgb565(83, 24, 31),
      };
  }

  return paletteFor(buddy::Expression::Neutral);
}

}  // namespace buddy_render
