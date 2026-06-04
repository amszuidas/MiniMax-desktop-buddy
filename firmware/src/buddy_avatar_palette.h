#pragma once
#ifndef UNIT_TEST
#include <Avatar.h>
#include <ColorPalette.h>
#include "buddy_palette.h"

namespace buddy_face {

inline void applyBuddyPalette(m5avatar::Avatar& avatar, buddy::Expression expr) {
  buddy_render::BuddyPalette p = buddy_render::paletteFor(expr);
  m5avatar::ColorPalette cp = avatar.getColorPalette();
  cp.set(COLOR_BACKGROUND, p.background);
  cp.set(COLOR_PRIMARY, p.primary);
  cp.set(COLOR_SECONDARY, p.secondary);
  cp.set(COLOR_BALLOON_BACKGROUND, p.balloonBg);
  cp.set(COLOR_BALLOON_FOREGROUND, p.balloonFg);
  avatar.setColorPalette(cp);
}

}  // namespace buddy_face
#endif
