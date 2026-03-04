#pragma once

#include "math_utils.h"
#include "vector3f.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

// Vertex data for rasterization
struct VertexOut {
  Vector3f screenPos; // x, y: screen_coord, z: NDC Z [-1,1]
  Vector3f color;     // raw_color
  float u, v;         // texture coordinate
  float invW;         // 1/w

  Vector3f colorDivW; // color / w
  float zDivW;        // z / w
};

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

inline void drawTriangle(const VertexOut &v0, const VertexOut &v1,
                         const VertexOut &v2,
                         std::vector<uint32_t> &framebuffer,
                         DepthBuffer &depthBuffer, int width, int height) {
  // AABB
  int minX = static_cast<int>(
      std::floor(std::min({v0.screenPos.x, v1.screenPos.x, v2.screenPos.x})));
  int maxX = static_cast<int>(
      std::ceil(std::max({v0.screenPos.x, v1.screenPos.x, v2.screenPos.x})));
  int minY = static_cast<int>(
      std::floor(std::min({v0.screenPos.y, v1.screenPos.y, v2.screenPos.y})));
  int maxY = static_cast<int>(
      std::ceil(std::max({v0.screenPos.y, v1.screenPos.y, v2.screenPos.y})));

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

      Vector3f bc = barycentric2D(v0.screenPos, v1.screenPos, v2.screenPos,
                                  p); // barycentric coordinates
      if (!insideTriangle(bc))
        continue;

      // ==== perspective-correct interpolation ====
      float sumInvW = bc.x * v0.invW + bc.y * v1.invW + bc.z * v2.invW;
      // (λ0/w0 + λ1/w1 + λ2/w2)
      if (std::abs(sumInvW) < 1e-6f)
        continue;

      float corrFactor = 1.0f / sumInvW;

      // I = (λ0*I0/w0 + λ1*I1/w1 + λ2*I2/w2) / (λ0/w0 + λ1/w1 + λ2/w2)
      float z = (v0.screenPos.z * v0.invW * bc.x + //
                 v1.screenPos.z * v1.invW * bc.y + //
                 v2.screenPos.z * v2.invW * bc.z) *
                corrFactor;
      float depth = 0.5f * (z + 1.0f); // NDC Z [-1,1] -> [0,1]

      // ========

      if (depth < depthBuffer.get(x, y)) {

        Vector3f color =
            (v0.colorDivW * bc.x + v1.colorDivW * bc.y + v2.colorDivW * bc.z) *
            corrFactor;
        float u = (v0.u * v0.invW * bc.x + //
                   v1.u * v1.invW * bc.y + //
                   v2.u * v2.invW * bc.z) *
                  corrFactor;
        float v = (v0.v * v0.invW * bc.x + //
                   v1.v * v1.invW * bc.y + //
                   v2.v * v2.invW * bc.z) *
                  corrFactor;

        // 0-1 5 grid
        Vector3f uvColor = ((static_cast<int>(std::abs(std::floor(u * 5.0f))) +
                             static_cast<int>(std::abs(std::floor(v * 5.0f)))) %
                                2 ==
                            0)
                               ? Vector3f(1.0f, 1.0f, 1.0f)
                               : Vector3f(0.0f, 0.0f, 0.0f);

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