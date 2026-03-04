#pragma once
#include "core/vector3f.h"

// Transform NDC coordinates to screen coordinates
inline Vector3f viewportTransform(const Vector3f &ndc, int screenWidth,
                                  int screenHeight) {
  float x = (ndc.x + 1.0f) * 0.5f * screenWidth;
  float y = (1.0f - ndc.y) * 0.5f * screenHeight;
  return Vector3f(x, y, ndc.z);
}