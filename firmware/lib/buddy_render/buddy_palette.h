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
          rgb565(7, 13, 35),     rgb565(25, 36, 94),    rgb565(68, 76, 198),
          rgb565(8, 13, 32),     rgb565(15, 20, 48),    rgb565(207, 234, 255),
          rgb565(145, 132, 255), rgb565(79, 91, 228),   rgb565(232, 244, 255),
          rgb565(13, 21, 52),
      };
    case buddy::Expression::Neutral:
      return {
          rgb565(255, 244, 213), rgb565(214, 246, 226), rgb565(205, 247, 224),
          rgb565(11, 17, 25),    rgb565(5, 61, 65),     rgb565(255, 207, 112),
          rgb565(115, 223, 207), rgb565(172, 242, 222), rgb565(252, 255, 246),
          rgb565(15, 35, 38),
      };
    case buddy::Expression::Happy:
      return {
          rgb565(7, 128, 56),    rgb565(34, 186, 83),   rgb565(158, 246, 133),
          rgb565(8, 28, 20),     rgb565(2, 71, 44),     rgb565(255, 119, 127),
          rgb565(102, 224, 240), rgb565(119, 246, 112), rgb565(244, 255, 235),
          rgb565(12, 48, 29),
      };
    case buddy::Expression::Doubt:
      return {
          rgb565(255, 58, 47),   rgb565(255, 112, 69),  rgb565(255, 151, 103),
          rgb565(22, 13, 18),    rgb565(86, 29, 22),    rgb565(178, 46, 25),
          rgb565(255, 215, 54),  rgb565(255, 124, 67),  rgb565(255, 247, 228),
          rgb565(64, 25, 21),
      };
    case buddy::Expression::Dizzy:
      return {
          rgb565(71, 37, 160),   rgb565(107, 70, 203),  rgb565(182, 132, 245),
          rgb565(18, 12, 45),    rgb565(67, 43, 127),   rgb565(255, 211, 57),
          rgb565(214, 150, 255), rgb565(156, 101, 255), rgb565(250, 242, 255),
          rgb565(39, 26, 92),
      };
    case buddy::Expression::Love:
      return {
          rgb565(211, 42, 111),  rgb565(245, 78, 139),  rgb565(255, 166, 198),
          rgb565(25, 10, 20),    rgb565(83, 24, 42),    rgb565(255, 43, 117),
          rgb565(255, 196, 223), rgb565(255, 105, 172), rgb565(255, 243, 248),
          rgb565(76, 20, 45),
      };
    case buddy::Expression::Sad:
      return {
          rgb565(12, 72, 156),   rgb565(36, 111, 204),  rgb565(134, 195, 255),
          rgb565(8, 18, 38),     rgb565(18, 66, 134),   rgb565(94, 220, 255),
          rgb565(145, 176, 255), rgb565(78, 151, 246),  rgb565(241, 249, 255),
          rgb565(15, 39, 82),
      };
    case buddy::Expression::Angry:
      return {
          rgb565(188, 24, 20),   rgb565(249, 55, 23),   rgb565(255, 126, 37),
          rgb565(26, 10, 10),    rgb565(102, 24, 18),   rgb565(255, 197, 38),
          rgb565(255, 91, 55),   rgb565(255, 73, 37),   rgb565(255, 239, 215),
          rgb565(74, 18, 15),
      };
  }

  return paletteFor(buddy::Expression::Neutral);
}

}  // namespace buddy_render
