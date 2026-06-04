#include <unity.h>
#include "vibration.h"

using namespace buddy;

void test_idle_returns_zero() {
  VibrationPlayer v;
  TEST_ASSERT_EQUAL_UINT8(0, v.update(0));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(99999));
}

void test_buzz_single_segment() {
  VibrationPlayer v;
  v.play(VibrationPattern::Buzz, 1000);
  TEST_ASSERT_EQUAL_UINT8(180, v.update(1000));
  TEST_ASSERT_EQUAL_UINT8(180, v.update(1119));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(1120));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(2000));
}

void test_double_tap_multi_segment() {
  VibrationPlayer v;
  v.play(VibrationPattern::DoubleTap, 0);
  TEST_ASSERT_EQUAL_UINT8(160, v.update(0));
  TEST_ASSERT_EQUAL_UINT8(160, v.update(79));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(80));
  TEST_ASSERT_EQUAL_UINT8(160, v.update(160));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(240));
}

void test_long_buzz() {
  VibrationPlayer v;
  v.play(VibrationPattern::LongBuzz, 500);
  TEST_ASSERT_EQUAL_UINT8(220, v.update(600));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(900));
}

void test_play_none_stops() {
  VibrationPlayer v;
  v.play(VibrationPattern::Buzz, 0);
  TEST_ASSERT_EQUAL_UINT8(180, v.update(10));
  v.play(VibrationPattern::None, 50);
  TEST_ASSERT_EQUAL_UINT8(0, v.update(60));
}

void test_replay_resets_from_new_start() {
  VibrationPlayer v;
  v.play(VibrationPattern::Buzz, 0);
  TEST_ASSERT_EQUAL_UINT8(0, v.update(200));
  v.play(VibrationPattern::Buzz, 1000);
  TEST_ASSERT_EQUAL_UINT8(180, v.update(1050));
}

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_idle_returns_zero);
  RUN_TEST(test_buzz_single_segment);
  RUN_TEST(test_double_tap_multi_segment);
  RUN_TEST(test_long_buzz);
  RUN_TEST(test_play_none_stops);
  RUN_TEST(test_replay_resets_from_new_start);
  return UNITY_END();
}
