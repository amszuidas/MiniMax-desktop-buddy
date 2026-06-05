#pragma once

#include <cstdint>

#include "pet_state.h"

namespace buddy_render {

constexpr int16_t kConceptScreenW = 320;
constexpr int16_t kConceptScreenH = 240;
constexpr int16_t kConceptFaceX = 34;
constexpr int16_t kConceptFaceY = 42;
constexpr int16_t kConceptFaceW = 252;
constexpr int16_t kConceptFaceH = 150;
constexpr int16_t kConceptFaceRadius = 58;
constexpr int16_t kConceptFaceCx = kConceptFaceX + kConceptFaceW / 2;
constexpr int16_t kConceptEyeY = 104;
constexpr int16_t kEyeCenterOffsetX = 54;
constexpr int16_t kEyeRadius = 26;
constexpr int16_t kConceptMouthY = 156;
constexpr int16_t kAlertMarkX = 258;
constexpr int16_t kAlertMarkY = 16;
constexpr int16_t kSadTearX = kConceptFaceCx + kEyeCenterOffsetX + 2;

enum class ConceptDecor : uint16_t {
  None = 0,
  Moon = 1u << 0,
  Zzz = 1u << 1,
  Sparkles = 1u << 2,
  Bubbles = 1u << 3,
  Focus = 1u << 4,
  Sweat = 1u << 5,
  SpeedLines = 1u << 6,
  Alert = 1u << 7,
  Aura = 1u << 8,
  OrbitStars = 1u << 9,
  Waves = 1u << 10,
  Hearts = 1u << 11,
  Rain = 1u << 12,
  Tear = 1u << 13,
  Warning = 1u << 14,
  Bolts = 1u << 15,
};

constexpr ConceptDecor operator|(ConceptDecor a, ConceptDecor b) {
  return static_cast<ConceptDecor>(static_cast<uint16_t>(a) |
                                   static_cast<uint16_t>(b));
}

constexpr ConceptDecor operator&(ConceptDecor a, ConceptDecor b) {
  return static_cast<ConceptDecor>(static_cast<uint16_t>(a) &
                                   static_cast<uint16_t>(b));
}

constexpr bool hasDecor(ConceptDecor mask, ConceptDecor bits) {
  return (static_cast<uint16_t>(mask & bits) == static_cast<uint16_t>(bits));
}

struct ConceptExpressionLayout {
  int16_t faceX;
  int16_t faceY;
  int16_t faceW;
  int16_t faceH;
  int16_t faceRadius;
  int16_t eyeY;
  int16_t eyeRadius;
  int16_t mouthY;
  int16_t mouthWidth;
  int16_t mouthHeight;
  ConceptDecor decor;
};

constexpr ConceptExpressionLayout makeConceptLayout(
    int16_t eyeRadius, int16_t mouthWidth, int16_t mouthHeight,
    ConceptDecor decor) {
  return {kConceptFaceX,      kConceptFaceY, kConceptFaceW,
          kConceptFaceH,      kConceptFaceRadius,
          kConceptEyeY,       eyeRadius,     kConceptMouthY,
          mouthWidth,         mouthHeight,   decor};
}

constexpr ConceptDecor conceptDecorFor(buddy::Expression e) {
  switch (e) {
    case buddy::Expression::Sleepy:
      return ConceptDecor::Moon | ConceptDecor::Zzz;
    case buddy::Expression::Neutral:
      return ConceptDecor::Sparkles | ConceptDecor::Bubbles;
    case buddy::Expression::Happy:
      return ConceptDecor::Focus | ConceptDecor::Sweat |
             ConceptDecor::SpeedLines;
    case buddy::Expression::Doubt:
      return ConceptDecor::Alert | ConceptDecor::Aura;
    case buddy::Expression::Dizzy:
      return ConceptDecor::OrbitStars | ConceptDecor::Waves;
    case buddy::Expression::Love:
      return ConceptDecor::Hearts | ConceptDecor::Sparkles;
    case buddy::Expression::Sad:
      return ConceptDecor::Rain | ConceptDecor::Tear;
    case buddy::Expression::Angry:
      return ConceptDecor::Warning | ConceptDecor::Bolts;
  }
  return ConceptDecor::Sparkles | ConceptDecor::Bubbles;
}

constexpr ConceptExpressionLayout conceptLayoutFor(buddy::Expression e) {
  switch (e) {
    case buddy::Expression::Sleepy:
      return makeConceptLayout(26, 18, 14, conceptDecorFor(e));
    case buddy::Expression::Neutral:
      return makeConceptLayout(27, 30, 14, conceptDecorFor(e));
    case buddy::Expression::Happy:
      return makeConceptLayout(26, 38, 20, conceptDecorFor(e));
    case buddy::Expression::Doubt:
      return makeConceptLayout(26, 20, 16, conceptDecorFor(e));
    case buddy::Expression::Dizzy:
      return makeConceptLayout(26, 28, 16, conceptDecorFor(e));
    case buddy::Expression::Love:
      return makeConceptLayout(28, 38, 20, conceptDecorFor(e));
    case buddy::Expression::Sad:
      return makeConceptLayout(26, 32, 14, conceptDecorFor(e));
    case buddy::Expression::Angry:
      return makeConceptLayout(26, 40, 22, conceptDecorFor(e));
  }
  return makeConceptLayout(27, 30, 14,
                           ConceptDecor::Sparkles | ConceptDecor::Bubbles);
}

}  // namespace buddy_render
