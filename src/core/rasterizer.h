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
inline void drawTriangle(const Vector3f &v0, const Vector3f &v1,
                         const Vector3f &v2, const Vector3f &c0,
                         const Vector3f &c1, const Vector3f &c2,
                         std::vector<uint32_t> &framebuffer,
                         DepthBuffer &depthBuffer, int width, int height) {
  // AABB
  int minX = static_cast<int>(std::floor(std::min({v0.x, v1.x, v2.x})));
  int maxX = static_cast<int>(std::ceil(std::max({v0.x, v1.x, v2.x})));
  int minY = static_cast<int>(std::floor(std::min({v0.y, v1.y, v2.y})));
  int maxY = static_cast<int>(std::ceil(std::max({v0.y, v1.y, v2.y})));

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

      Vector3f bc = barycentric2D(v0, v1, v2, p); // barycentric coordinates
      if (!insideTriangle(bc))
        continue;

      // [TEST] linear interpolation of z
      float z = bc.x * v0.z + bc.y * v1.z + bc.z * v2.z;

      // [-1,1] -> [0,1]
      float depth = (z + 1.0f) * 0.5f;

      if (depth < depthBuffer.get(x, y)) {
        // [TEST] linear interpolation of color
        Vector3f color = c0 * bc.x + c1 * bc.y + c2 * bc.z;

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