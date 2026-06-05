#include <unity.h>
#include "buddy_layout.h"
#include "buddy_palette.h"

using buddy::Expression;
using buddy_render::ConceptDecor;
using buddy_render::ConceptExpressionLayout;
using buddy_render::BuddyPalette;
using buddy_render::blend565;
using buddy_render::conceptDecorFor;
using buddy_render::conceptLayoutFor;
using buddy_render::hasDecor;
using buddy_render::kAvatarColorDepth;
using buddy_render::kConceptFaceH;
using buddy_render::kConceptFaceW;
using buddy_render::kConceptScreenH;
using buddy_render::kConceptScreenW;
using buddy_render::kEyeCenterOffsetX;
using buddy_render::kEyeRadius;
using buddy_render::kAlertMarkX;
using buddy_render::kSadTearX;
using buddy_render::blue8;
using buddy_render::green8;
using buddy_render::paletteFor;
using buddy_render::red8;
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

void test_concept_face_matches_keyframe_scale() {
  TEST_ASSERT_EQUAL(320, kConceptScreenW);
  TEST_ASSERT_EQUAL(240, kConceptScreenH);
  TEST_ASSERT_GREATER_OR_EQUAL(220, kConceptFaceW);
  TEST_ASSERT_GREATER_OR_EQUAL(136, kConceptFaceH);
  TEST_ASSERT_GREATER_OR_EQUAL(48, kEyeCenterOffsetX);
  TEST_ASSERT_GREATER_OR_EQUAL(22, kEyeRadius);
  TEST_ASSERT_GREATER_THAN(230, kAlertMarkX);
  TEST_ASSERT_GREATER_THAN(200, kSadTearX);
}

void test_all_expressions_have_concept_layouts() {
  const Expression expressions[] = {
      Expression::Sleepy, Expression::Neutral, Expression::Happy,
      Expression::Doubt, Expression::Dizzy, Expression::Love,
      Expression::Sad, Expression::Angry,
  };

  for (Expression e : expressions) {
    ConceptExpressionLayout layout = conceptLayoutFor(e);
    TEST_ASSERT_GREATER_OR_EQUAL(26, layout.eyeRadius);
    TEST_ASSERT_GREATER_OR_EQUAL(12, layout.mouthHeight);
    TEST_ASSERT_NOT_EQUAL(ConceptDecor::None, conceptDecorFor(e));
  }
}

void test_decorations_follow_approved_keyframes() {
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Sleepy),
                            ConceptDecor::Moon | ConceptDecor::Zzz));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Neutral),
                            ConceptDecor::Sparkles | ConceptDecor::Bubbles));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Happy),
                            ConceptDecor::Focus | ConceptDecor::Sweat |
                                ConceptDecor::SpeedLines));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Doubt),
                            ConceptDecor::Alert | ConceptDecor::Aura));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Dizzy),
                            ConceptDecor::OrbitStars | ConceptDecor::Waves));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Love),
                            ConceptDecor::Hearts | ConceptDecor::Sparkles));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Sad),
                            ConceptDecor::Rain | ConceptDecor::Tear));
  TEST_ASSERT_TRUE(hasDecor(conceptDecorFor(Expression::Angry),
                            ConceptDecor::Warning | ConceptDecor::Bolts));
}

void test_keyframe_palettes_use_dark_ink_lines() {
  const Expression expressions[] = {
      Expression::Sleepy, Expression::Neutral, Expression::Happy,
      Expression::Doubt, Expression::Dizzy, Expression::Love,
      Expression::Sad, Expression::Angry,
  };

  for (Expression e : expressions) {
    BuddyPalette p = paletteFor(e);
    TEST_ASSERT_LESS_THAN_UINT8(80, red8(p.primary));
    TEST_ASSERT_LESS_THAN_UINT8(80, green8(p.primary));
    TEST_ASSERT_LESS_THAN_UINT8(96, blue8(p.primary));
  }
}

void test_keyframe_face_colors_match_expression_hues() {
  BuddyPalette sleepy = paletteFor(Expression::Sleepy);
  TEST_ASSERT_GREATER_THAN_UINT8(red8(sleepy.face), blue8(sleepy.face));
  TEST_ASSERT_GREATER_THAN_UINT8(green8(sleepy.face), blue8(sleepy.face));

  BuddyPalette neutral = paletteFor(Expression::Neutral);
  TEST_ASSERT_GREATER_OR_EQUAL_UINT8(red8(neutral.face), green8(neutral.face));

  BuddyPalette happy = paletteFor(Expression::Happy);
  TEST_ASSERT_GREATER_THAN_UINT8(red8(happy.face), green8(happy.face));

  BuddyPalette doubt = paletteFor(Expression::Doubt);
  TEST_ASSERT_GREATER_THAN_UINT8(blue8(doubt.face), red8(doubt.face));

  BuddyPalette dizzy = paletteFor(Expression::Dizzy);
  TEST_ASSERT_GREATER_THAN_UINT8(green8(dizzy.face), blue8(dizzy.face));

  BuddyPalette love = paletteFor(Expression::Love);
  TEST_ASSERT_GREATER_THAN_UINT8(green8(love.face), red8(love.face));

  BuddyPalette sad = paletteFor(Expression::Sad);
  TEST_ASSERT_GREATER_THAN_UINT8(red8(sad.face), blue8(sad.face));

  BuddyPalette angry = paletteFor(Expression::Angry);
  TEST_ASSERT_GREATER_THAN_UINT8(blue8(angry.face), red8(angry.face));
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
  RUN_TEST(test_concept_face_matches_keyframe_scale);
  RUN_TEST(test_all_expressions_have_concept_layouts);
  RUN_TEST(test_decorations_follow_approved_keyframes);
  RUN_TEST(test_keyframe_palettes_use_dark_ink_lines);
  RUN_TEST(test_keyframe_face_colors_match_expression_hues);
  return UNITY_END();
}
