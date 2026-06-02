#ifndef UNIT_TEST
#include <M5Unified.h>
#include <Avatar.h>

using namespace m5avatar;

Avatar avatar;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setBrightness(120);
  avatar.init();  // avatar 在独立线程渲染,无需在 loop 里逐帧画
}

void loop() {
  M5.update();
  delay(16);
}
#endif
