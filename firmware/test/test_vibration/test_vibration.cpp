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

void test_pulse_pattern() {
  VibrationPlayer v;
  v.play(VibrationPattern::Pulse, 0);
  TEST_ASSERT_EQUAL_UINT8(140, v.update(0));
  TEST_ASSERT_EQUAL_UINT8(140, v.update(99));
  TEST_ASSERT_EQUAL_UINT8(0, v.update(100));
}

void test_mid_play_preemption() {
  VibrationPlayer v;
  v.play(VibrationPattern::DoubleTap, 0);
  TEST_ASSERT_EQUAL_UINT8(160, v.update(40));   // DoubleTap seg0 进行中
  v.play(VibrationPattern::Buzz, 50);            // 中途打断换 Buzz
  TEST_ASSERT_EQUAL_UINT8(180, v.update(60));    // 新模式从 50 起算
  TEST_ASSERT_EQUAL_UINT8(0, v.update(170));     // 50+120 后结束
}

void test_wraparound_safe() {
  VibrationPlayer v;
  // 在接近 uint32 上限处起播,now 回绕过 0 仍应正确(elapsed 无符号差值)
  v.play(VibrationPattern::Buzz, 0xFFFFFFF0u);   // start 接近上限
  TEST_ASSERT_EQUAL_UINT8(180, v.update(0x00000010u)); // elapsed=0x20=32ms < 120,仍在段内
  TEST_ASSERT_EQUAL_UINT8(0, v.update(0x00000070u));   // elapsed=0x80=128ms > 120,结束
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
  RUN_TEST(test_pulse_pattern);
  RUN_TEST(test_mid_play_preemption);
  RUN_TEST(test_wraparound_safe);
  return UNITY_END();
}
