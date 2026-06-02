#include <unity.h>
#include "pet_state.h"

using namespace buddy;

static PetVisual atSteady(const PetInputs& in) {
  PetStateMachine sm;
  sm.setInputs(in);
  return sm.update(1000);  // 任意 now,无瞬态
}

// ---- 稳态优先级 ----

void test_disconnected_is_sleepy() {
  PetVisual v = atSteady({.connected = false, .runningSessions = 0, .pendingApprovals = 0});
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Sleepy), static_cast<int>(v.expression));
  TEST_ASSERT_EQUAL(static_cast<int>(Led::Off), static_cast<int>(v.led));
}

void test_connected_idle_is_neutral() {
  PetVisual v = atSteady({.connected = true, .runningSessions = 0, .pendingApprovals = 0});
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Neutral), static_cast<int>(v.expression));
}

void test_running_is_happy() {
  PetVisual v = atSteady({.connected = true, .runningSessions = 2, .pendingApprovals = 0});
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Happy), static_cast<int>(v.expression));
  TEST_ASSERT_EQUAL(static_cast<int>(Led::GreenBreath), static_cast<int>(v.led));
}

void test_pending_beats_running() {
  PetVisual v = atSteady({.connected = true, .runningSessions = 3, .pendingApprovals = 1});
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Doubt), static_cast<int>(v.expression));
  TEST_ASSERT_EQUAL(static_cast<int>(Led::RedBlink), static_cast<int>(v.led));
  TEST_ASSERT_EQUAL(static_cast<int>(Vibration::Pulse), static_cast<int>(v.vibration));
}

void test_disconnected_beats_pending_failsafe() {
  // 断连即使 store 还报 pending,也必须显示未连接(数据不可信)
  PetVisual v = atSteady({.connected = false, .runningSessions = 0, .pendingApprovals = 5});
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Sleepy), static_cast<int>(v.expression));
}

// ---- 瞬态触发 + 衰减 ----

void test_shake_shows_dizzy_then_falls_back() {
  PetStateMachine sm;
  sm.setInputs({.connected = true, .runningSessions = 0, .pendingApprovals = 0});
  sm.onShake(1000);
  // 摇晃期间:Dizzy
  PetVisual during = sm.update(1500);
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Dizzy), static_cast<int>(during.expression));
  TEST_ASSERT_EQUAL(static_cast<int>(Led::RainbowSpin), static_cast<int>(during.led));
  // 2s 后回落到稳态(idle→Neutral)
  PetVisual after = sm.update(1000 + kShakeMs + 1);
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Neutral), static_cast<int>(after.expression));
}

void test_approve_shows_love_then_falls_back() {
  PetStateMachine sm;
  sm.setInputs({.connected = true, .runningSessions = 1, .pendingApprovals = 0});
  sm.onApprove(2000);
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Love),
                    static_cast<int>(sm.update(2200).expression));
  // 回落到 running→Happy
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Happy),
                    static_cast<int>(sm.update(2000 + kApproveMs + 1).expression));
}

void test_deny_shows_sad() {
  PetStateMachine sm;
  sm.setInputs({.connected = true});
  sm.onDeny(500);
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Sad),
                    static_cast<int>(sm.update(700).expression));
  // 过期后回落到 idle→Neutral
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Neutral),
                    static_cast<int>(sm.update(500 + kDenyMs + 1).expression));
}

void test_error_shows_angry_with_longbuzz() {
  PetStateMachine sm;
  sm.setInputs({.connected = true});
  sm.onSessionError(0);
  PetVisual v = sm.update(100);
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Angry), static_cast<int>(v.expression));
  TEST_ASSERT_EQUAL(static_cast<int>(Vibration::LongBuzz), static_cast<int>(v.vibration));
  // 过期后回落到 idle→Neutral
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Neutral),
                    static_cast<int>(sm.update(0 + kErrorMs + 1).expression));
}

void test_shake_beats_approve_when_both_active() {
  PetStateMachine sm;
  sm.setInputs({.connected = true});
  sm.onApprove(1000);
  sm.onShake(1000);  // 同时活跃,摇晃优先级更高
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Dizzy),
                    static_cast<int>(sm.update(1200).expression));
}

void test_error_beats_approve_when_both_active() {
  PetStateMachine sm;
  sm.setInputs({.connected = true});
  sm.onApprove(1000);
  sm.onSessionError(1000);  // 同时活跃,Error 优先级高于 Approve
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Angry),
                    static_cast<int>(sm.update(1200).expression));
}

void test_shake_works_even_when_disconnected() {
  PetStateMachine sm;
  sm.setInputs({.connected = false});  // 断连
  sm.onShake(1000);
  TEST_ASSERT_EQUAL(static_cast<int>(Expression::Dizzy),
                    static_cast<int>(sm.update(1200).expression));
}

// ---- Unity 入口 ----
void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_disconnected_is_sleepy);
  RUN_TEST(test_connected_idle_is_neutral);
  RUN_TEST(test_running_is_happy);
  RUN_TEST(test_pending_beats_running);
  RUN_TEST(test_disconnected_beats_pending_failsafe);
  RUN_TEST(test_shake_shows_dizzy_then_falls_back);
  RUN_TEST(test_approve_shows_love_then_falls_back);
  RUN_TEST(test_deny_shows_sad);
  RUN_TEST(test_error_shows_angry_with_longbuzz);
  RUN_TEST(test_shake_beats_approve_when_both_active);
  RUN_TEST(test_error_beats_approve_when_both_active);
  RUN_TEST(test_shake_works_even_when_disconnected);
  return UNITY_END();
}
