#include "vibration.h"

namespace buddy {

namespace {
struct Seg { uint8_t level; uint16_t durMs; };

const Seg kBuzz[] = {{180, 120}};
const Seg kDoubleTap[] = {{160, 80}, {0, 80}, {160, 80}};
const Seg kLongBuzz[] = {{220, 400}};
const Seg kPulse[] = {{140, 100}};

void scriptFor(VibrationPattern p, const Seg*& segs, int& count) {
  switch (p) {
    case VibrationPattern::Buzz:      segs = kBuzz;      count = 1; break;
    case VibrationPattern::DoubleTap: segs = kDoubleTap; count = 3; break;
    case VibrationPattern::LongBuzz:  segs = kLongBuzz;  count = 1; break;
    case VibrationPattern::Pulse:     segs = kPulse;     count = 1; break;
    case VibrationPattern::None:      segs = nullptr;    count = 0; break;
  }
}
}  // namespace

void VibrationPlayer::play(VibrationPattern pattern, uint32_t now_ms) {
  pattern_ = pattern;
  startMs_ = now_ms;
  playing_ = (pattern != VibrationPattern::None);
}

uint8_t VibrationPlayer::update(uint32_t now_ms) const {
  if (!playing_) return 0;
  const Seg* segs = nullptr;
  int count = 0;
  scriptFor(pattern_, segs, count);
  if (count == 0) return 0;
  uint32_t elapsed = now_ms - startMs_;
  uint32_t acc = 0;
  for (int i = 0; i < count; i++) {
    uint32_t end = acc + segs[i].durMs;
    if (elapsed < end) return segs[i].level;
    acc = end;
  }
  return 0;
}

}  // namespace buddy
