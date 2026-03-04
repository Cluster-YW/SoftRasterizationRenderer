#pragma once
#include "vector3f.h"

// compute barycentric coordinates of a 2D point in a triangle
// 计算重心坐标
inline Vector3f barycentric2D(const Vector3f &v1, const Vector3f &v2,
                              const Vector3f &v3, const Vector3f &p) {
  // compute area of the triangle
  float area = (v2.x - v1.x) * (v3.y - v1.y) - (v3.x - v1.x) * (v2.y - v1.y);

  // check if the triangle is degenerate
  if (std::abs(area) < 1e-6f) {
    return Vector3f(-1.0f, -1.0f,
                    -1.0f); // return invalid barycentric coordinates
  }

  // compute barycentric coordinates
  float b1 = ((v2.x - p.x) * (v3.y - p.y) - (v3.x - p.x) * (v2.y - p.y)) / area;
  float b2 = ((v3.x - p.x) * (v1.y - p.y) - (v1.x - p.x) * (v3.y - p.y)) / area;
  float b3 = 1.0f - b1 - b2;

  return Vector3f(b1, b2, b3);
}

inline bool insideTriangle(const Vector3f &bc) {
  return bc.x >= 0.0f && bc.y >= 0.0f && bc.z >= 0.0f &&
         bc.x + bc.y + bc.z <= 1.0f;
}