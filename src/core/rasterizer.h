#pragma once

#include "math_utils.h"
#include "vector3f.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

class DepthBuffer {
public:
  int width, height;
  std::vector<float> buffer;

  DepthBuffer(int width, int height) : width(width), height(height) {
    buffer.resize(width * height);
    clear();
  }

  void clear() { std::fill(buffer.begin(), buffer.end(), 1.0f); }

  float get(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height)
      return 1.0f;
    return buffer[y * width + x];
  }
  void set(int x, int y, float z) {
    if (x < 0 || x >= width || y < 0 || y >= height)
      return;
    buffer[y * width + x] = z;
  }
};
inline void drawTriangle(const Vector3f &p0, const Vector3f &p1,
                         const Vector3f &p2, const Vector3f &c0,
                         const Vector3f &c1, const Vector3f &c2, float u0,
                         float v0, float u1, float v1, float u2, float v2,
                         std::vector<uint32_t> &framebuffer,
                         DepthBuffer &depthBuffer, int width, int height) {
  // AABB
  int minX = static_cast<int>(std::floor(std::min({p0.x, p1.x, p2.x})));
  int maxX = static_cast<int>(std::ceil(std::max({p0.x, p1.x, p2.x})));
  int minY = static_cast<int>(std::floor(std::min({p0.y, p1.y, p2.y})));
  int maxY = static_cast<int>(std::ceil(std::max({p0.y, p1.y, p2.y})));

  // clip to screen
  minX = std::max(minX, 0);
  maxX = std::min(maxX, width - 1);
  minY = std::max(minY, 0);
  maxY = std::min(maxY, height - 1);

  // for each pixel in the AABB
  for (int x = minX; x <= maxX; ++x) {
    for (int y = minY; y <= maxY; ++y) {
      // pixel center
      Vector3f p(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f,
                 0.0f);

      Vector3f bc = barycentric2D(p0, p1, p2, p); // barycentric coordinates
      if (!insideTriangle(bc))
        continue;

      // [TEST] linear interpolation of z
      float z = bc.x * p0.z + bc.y * p1.z + bc.z * p2.z;

      // [-1,1] -> [0,1]
      float depth = (z + 1.0f) * 0.5f;

      if (depth < depthBuffer.get(x, y)) {
        // [TEST] linear interpolation of uv
        float u = bc.dot(Vector3f(u0, u1, u2));
        float v = bc.dot(Vector3f(v0, v1, v2));

        // [TEST] Chessboarder pattern
        // 0-1 5 grid
        Vector3f uvColor = ((static_cast<int>(std::abs(std::floor(u * 5.0f))) +
                             static_cast<int>(std::abs(std::floor(v * 5.0f)))) %
                                2 ==
                            0)
                               ? Vector3f(1.0f, 1.0f, 1.0f)
                               : Vector3f(0.0f, 0.0f, 0.0f);

        // [TEST] bilinear interpolation of color
        Vector3f color = c0 * bc.x + c1 * bc.y + c2 * bc.z;
        color = uvColor * 0.5f + color * 0.5f;

        uint8_t r = static_cast<uint8_t>(color.x * 255.0f);
        uint8_t g = static_cast<uint8_t>(color.y * 255.0f);
        uint8_t b = static_cast<uint8_t>(color.z * 255.0f);
        uint32_t pixel = 0xFF000000 | (r << 16) | (g << 8) | b; // ARGB

        framebuffer[y * width + x] = pixel;
        depthBuffer.set(x, y, depth);
      }
    }
  }
}