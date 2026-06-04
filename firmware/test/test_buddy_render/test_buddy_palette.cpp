#include <unity.h>
#include "buddy_palette.h"

using buddy::Expression;
using buddy_render::BuddyPalette;
using buddy_render::blend565;
using buddy_render::kAvatarColorDepth;
using buddy_render::paletteFor;
using buddy_render::rgb565;
using buddy_render::sweatCountForIntensity;
using buddy_render::triWave8;

void test_avatar_color_depth_supports_rgb_palettes() {
  TEST_ASSERT_EQUAL(16, kAvatarColorDepth);
}

void test_rgb565_known_values() {
  TEST_ASSERT_EQUAL_UINT16(0x0000, rgb565(0, 0, 0));
  TEST_ASSERT_EQUAL_UINT16(0xFFFF, rgb565(255, 255, 255));
  TEST_ASSERT_EQUAL_UINT16(0xF800, rgb565(255, 0, 0));
  TEST_ASSERT_EQUAL_UINT16(0x07E0, rgb565(0, 255, 0));
  TEST_ASSERT_EQUAL_UINT16(0x001F, rgb565(0, 0, 255));
}

void test_all_expression_backgrounds_are_colorful() {
  const Expression expressions[] = {
      Expression::Sleepy, Expression::Neutral, Expression::Happy,
      Expression::Doubt, Expression::Dizzy, Expression::Love,
      Expression::Sad, Expression::Angry,
  };
  for (Expression e : expressions) {
    BuddyPalette p = paletteFor(e);
    TEST_ASSERT_NOT_EQUAL_UINT16(0x0000, p.background);
    TEST_ASSERT_NOT_EQUAL_UINT16(p.background, p.primary);
    TEST_ASSERT_NOT_EQUAL_UINT16(p.background, p.accent);
  }
}

void test_pending_and_error_palettes_are_distinct() {
  BuddyPalette pending = paletteFor(Expression::Doubt);
  BuddyPalette error = paletteFor(Expression::Angry);
  TEST_ASSERT_NOT_EQUAL_UINT16(pending.background, error.background);
  TEST_ASSERT_NOT_EQUAL_UINT16(pending.accent, error.accent);
  TEST_ASSERT_NOT_EQUAL_UINT16(pending.glow, error.glow);
}

void test_tri_wave_reaches_edges_and_midpoint() {
  TEST_ASSERT_EQUAL_UINT8(0, triWave8(0, 1000));
  TEST_ASSERT_EQUAL_UINT8(127, triWave8(250, 1000));
  TEST_ASSERT_EQUAL_UINT8(255, triWave8(500, 1000));
  TEST_ASSERT_EQUAL_UINT8(128, triWave8(750, 1000));
  TEST_ASSERT_EQUAL_UINT8(0, triWave8(1000, 1000));
}

void test_blend565_endpoints() {
  uint16_t red = rgb565(255, 0, 0);
  uint16_t blue = rgb565(0, 0, 255);
  TEST_ASSERT_EQUAL_UINT16(red, blend565(red, blue, 0));
  TEST_ASSERT_EQUAL_UINT16(blue, blend565(red, blue, 255));
}

void test_sweat_count_for_workload_intensity() {
  TEST_ASSERT_EQUAL(1, sweatCountForIntensity(0));
  TEST_ASSERT_EQUAL(1, sweatCountForIntensity(80));
  TEST_ASSERT_EQUAL(2, sweatCountForIntensity(160));
  TEST_ASSERT_EQUAL(3, sweatCountForIntensity(240));
  TEST_ASSERT_EQUAL(3, sweatCountForIntensity(255));
}

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_avatar_color_depth_supports_rgb_palettes);
  RUN_TEST(test_rgb565_known_values);
  RUN_TEST(test_all_expression_backgrounds_are_colorful);
  RUN_TEST(test_pending_and_error_palettes_are_distinct);
  RUN_TEST(test_tri_wave_reaches_edges_and_midpoint);
  RUN_TEST(test_blend565_endpoints);
  RUN_TEST(test_sweat_count_for_workload_intensity);
  return UNITY_END();
}
