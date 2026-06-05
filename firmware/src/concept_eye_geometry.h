#pragma once
#ifndef UNIT_TEST
#include <Drawable.h>
#include "buddy_layout.h"

namespace buddy_face {

inline bool isLeftConceptSocket(m5avatar::BoundingRect rect) {
  return rect.getCenterX() < buddy_render::kConceptFaceCx;
}

inline int16_t conceptEyeX(bool isLeft) {
  return buddy_render::kConceptFaceCx +
         (isLeft ? -buddy_render::kEyeCenterOffsetX
                 : buddy_render::kEyeCenterOffsetX);
}

inline int16_t conceptEyeX(m5avatar::BoundingRect rect) {
  return conceptEyeX(isLeftConceptSocket(rect));
}

inline int16_t conceptEyeY() {
  return buddy_render::kConceptEyeY;
}

}  // namespace buddy_face
#endif
