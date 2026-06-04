#pragma once
#ifndef UNIT_TEST
#include <Face.h>
#include <Eye.h>
#include <Mouth.h>
#include <Eyeblow.h>
#include "spiral_eye.h"
#include "heart_eye.h"
#include "alert_eye.h"
#include "busy_eye.h"
#include "sad_eye.h"
#include "angry_eye.h"
#include "buddy_effect.h"

// Custom Face assembly for buddy desktop pet.
//
// Design decisions:
//   - Per-expression custom eyes: swap left/right Eye Drawables via
//     Face::setLeftEye() / setRightEye().  setEyeKind(face, kind) installs the
//     SpiralEye instances (Dizzy), the HeartEye instances (Love), or restores
//     the default library Eye instances (everything else).
//
//   - Overlay effects (hearts / sweat / Zzz / stars): the library Face has no
//     setEffect().  addTask() gives no canvas access.  So we use the Mouth slot
//     via Face::setMouth(): BuddyEffect wraps the real Mouth Drawable, draws
//     the mouth first, then overlays effect symbols.  The mouth is never lost.
//
// All custom Drawables read animation state from the global g_buddyFx struct.

namespace buddy_face {

// Persistent Drawable instances (static lifetime).
// The library Face stores raw pointers but never deletes parts set via setters
// (only the initial ones from the constructor or the ones assigned at
// construction time get cleaned up in ~Face).  Since we swap in/out, we own
// these ourselves with static lifetime.
inline SpiralEye s_spiralL;
inline SpiralEye s_spiralR;
inline HeartEye s_heartL;
inline HeartEye s_heartR;
inline AlertEye s_alertL(true);
inline AlertEye s_alertR(false);
inline BusyEye s_busyL;
inline BusyEye s_busyR;
inline SadEye s_sadL(true);
inline SadEye s_sadR(false);
inline AngryEye s_angryL(true);
inline AngryEye s_angryR(false);
inline m5avatar::Eye s_defaultEyeR(8, false);
inline m5avatar::Eye s_defaultEyeL(8, true);

// Which custom eyes are currently installed.
enum class EyeKind { Default, Spiral, Heart, Alert, Busy, Sad, Angry };
inline EyeKind s_curEye = EyeKind::Default;

// Swap eyes to the requested kind (no-op if already installed).
inline void setEyeKind(m5avatar::Face* face, EyeKind kind) {
  if (kind == s_curEye) return;
  switch (kind) {
    case EyeKind::Spiral:
      face->setLeftEye(&s_spiralL);
      face->setRightEye(&s_spiralR);
      break;
    case EyeKind::Heart:
      face->setLeftEye(&s_heartL);
      face->setRightEye(&s_heartR);
      break;
    case EyeKind::Alert:
      face->setLeftEye(&s_alertL);
      face->setRightEye(&s_alertR);
      break;
    case EyeKind::Busy:
      face->setLeftEye(&s_busyL);
      face->setRightEye(&s_busyR);
      break;
    case EyeKind::Sad:
      face->setLeftEye(&s_sadL);
      face->setRightEye(&s_sadR);
      break;
    case EyeKind::Angry:
      face->setLeftEye(&s_angryL);
      face->setRightEye(&s_angryR);
      break;
    case EyeKind::Default:
    default:
      face->setLeftEye(&s_defaultEyeL);
      face->setRightEye(&s_defaultEyeR);
      break;
  }
  s_curEye = kind;
}

// Create a Face with BuddyEffect installed in the mouth slot.
// Returns a heap-allocated Face* suitable for Avatar::setFace().
// The returned Face owns its constructor-allocated parts; the BuddyEffect
// wrapping is set via setMouth() and has static lifetime.
inline m5avatar::Face* makeBuddyFace() {
  // Use the library default Face layout, then replace the mouth slot.
  auto* face = new m5avatar::Face();

  // The default Face constructor creates a Mouth(50, 90, 4, 60).
  // We wrap it with BuddyEffect.  Since Face::setMouth() just replaces the
  // pointer (doesn't delete the old one -- the constructor-allocated Mouth is
  // deleted in ~Face), we need our own Mouth for delegation.
  static m5avatar::Mouth s_mouth(50, 90, 4, 60);
  static BuddyEffect s_effect(&s_mouth);
  // 注意:~Face() 会对装入的 mouth 指针 delete。这里装的是函数内 static BuddyEffect,
  // 仅因本固件永不析构 Face(MCU 直接 reset)而安全。若将来支持优雅关闭/换脸,需改。
  // 同理 new Face() 默认分配的 Mouth 被 setMouth 替换后泄漏(一次性 ~20 字节,可接受)。
  face->setMouth(&s_effect);

  return face;
}

}  // namespace buddy_face
#endif
