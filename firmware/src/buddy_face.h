#pragma once
#ifndef UNIT_TEST
#include <Face.h>
#include <Eye.h>
#include <Mouth.h>
#include <Eyeblow.h>
#include "spiral_eye.h"
#include "buddy_effect.h"

// Custom Face assembly for buddy desktop pet.
//
// Design decisions:
//   - Spiral eyes (Dizzy): swap left/right Eye Drawables via
//     Face::setLeftEye() / setRightEye().  setDizzyEyes(face, true) installs
//     SpiralEye instances; false restores the default library Eye instances.
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
inline m5avatar::Eye s_defaultEyeR(8, false);
inline m5avatar::Eye s_defaultEyeL(8, true);

// Track whether spiral eyes are currently installed.
inline bool s_dizzyActive = false;

// Swap eyes between spiral and default.
inline void setDizzyEyes(m5avatar::Face* face, bool dizzy) {
  if (dizzy == s_dizzyActive) return;
  if (dizzy) {
    face->setLeftEye(&s_spiralL);
    face->setRightEye(&s_spiralR);
  } else {
    face->setLeftEye(&s_defaultEyeL);
    face->setRightEye(&s_defaultEyeR);
  }
  s_dizzyActive = dizzy;
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
