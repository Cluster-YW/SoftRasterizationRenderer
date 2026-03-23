#pragma once

#include "math/math_utils.h"
#include "math/vector2f.h"
#include "math/vector3f.h"
#include "render/shader.h"
#include "resource/texture.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

using Framebuffer = std::vector<uint32_t>;

using namespace sr::math;
namespace sr {
namespace render {

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

void put_pixel(Framebuffer &framebuffer, int x, int y, uint32_t color) {
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    return;
  framebuffer[y * SCREEN_WIDTH + x] = color;
}
void draw_line(Framebuffer &framebuffer, int x0, int y0, int x1, int y1,
               uint32_t color) {
  bool steep =
      std::abs(y1 - y0) > std::abs(x1 - x0); // determine if the line is steep
  if (steep) { // if the line is steep, swap the x and y coordinates
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  // ensure x0 <= x1
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // Bresenham's algorithm
  int dx = x1 - x0;
  int dy = std::abs(y1 - y0);
  int err = dx / 2;
  int ystep = (y0 < y1) ? 1 : -1;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      put_pixel(framebuffer, y, x, color);
    } else {
      put_pixel(framebuffer, x, y, color);
    }
    err -= dy;
    if (err < 0) {
      y += ystep;
      err += dx;
    }
  }
}

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

inline void drawTriangle(const Varying &v0, const Varying &v1,
                         const Varying &v2, //
                         const ShaderProgram &shader, const Uniforms &uniforms,
                         std::vector<uint32_t> &framebuffer,
                         DepthBuffer &depthBuffer) {

  int minY = static_cast<int>(
      std::floor(std::min({v0.screenPos.y, v1.screenPos.y, v2.screenPos.y})));
  int maxY = static_cast<int>(
      std::ceil(std::max({v0.screenPos.y, v1.screenPos.y, v2.screenPos.y})));
  minY = std::max(minY, 0);
  maxY = std::min(maxY, uniforms.screenHeight - 1);

  auto intersectX = [](const Vector3f &a, const Vector3f &b,
                       float yf) -> float {
    float dy = b.y - a.y;
    if (std::abs(dy) < 1e-6f) // skip horizontal edges
      return std::numeric_limits<float>::quiet_NaN();

    if (yf < a.y && yf < b.y || yf > a.y && yf > b.y) // out of range
      return std::numeric_limits<float>::quiet_NaN();

    float t = (yf - a.y) / dy;
    return a.x + t * (b.x - a.x);
  };

  // for each pixel in the AABB
  for (int y = minY; y <= maxY; ++y) {
    float yf = static_cast<float>(y) + 0.5f;

    float xs[3];
    int xCount = 0;

    float x0 = intersectX(v0.screenPos, v1.screenPos, yf);
    if (!std::isnan(x0))
      xs[xCount++] = x0;

    float x1 = intersectX(v1.screenPos, v2.screenPos, yf);
    if (!std::isnan(x1))
      xs[xCount++] = x1;

    float x2 = intersectX(v2.screenPos, v0.screenPos, yf);
    if (!std::isnan(x2))
      xs[xCount++] = x2;

    if (xCount < 2)
      continue; // skip degenerate triangles

    float xStart = xs[0];
    float xEnd = xs[0];
    for (int i = 1; i < xCount; ++i) {
      xStart = std::min(xStart, xs[i]);
      xEnd = std::max(xEnd, xs[i]);
    }

    int minX = static_cast<int>(std::ceil(xStart - 0.5f));
    int maxX = static_cast<int>(std::floor(xEnd - 0.5f));
    minX = std::max(minX, 0);
    maxX = std::min(maxX, uniforms.screenWidth - 1);

    for (int x = minX; x <= maxX; ++x) {
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
      float z = (v0.screenPos.z * v0.invW * bc.x +  //
                 v1.screenPos.z * v1.invW * bc.y +  //
                 v2.screenPos.z * v2.invW * bc.z) * //
                corrFactor;
      float depth = 0.5f * (z + 1.0f); // NDC Z [-1,1] -> [0,1]

      if (depth >= depthBuffer.get(x, y)) // early depth test
        continue;

      Varying interp;
      Vector3f factor(v0.invW, v1.invW, v2.invW);
      factor = factor.product(bc) * corrFactor;
      interp.position =
          factor.interpolate(v0.position, v1.position, v2.position);
      interp.normal = factor.interpolate(v0.normal, v1.normal, v2.normal);
      interp.screenPos =
          factor.interpolate(v0.screenPos, v1.screenPos, v2.screenPos);
      interp.texcoord =
          factor.interpolate(v0.texcoord, v1.texcoord, v2.texcoord);
      interp.color = factor.interpolate(v0.color, v1.color, v2.color);

      // mix light and albedo
      Vector3f finalColor;
      finalColor = shader.fragmentShader(interp, uniforms);

      finalColor = finalColor.clamp(0, 1.0f);
      uint8_t r = static_cast<uint8_t>(finalColor.x * 255.0f);
      uint8_t g = static_cast<uint8_t>(finalColor.y * 255.0f);
      uint8_t b = static_cast<uint8_t>(finalColor.z * 255.0f);
      framebuffer[y * uniforms.screenWidth + x] =
          0xFF000000 | (r << 16) | (g << 8) | b;

      depthBuffer.set(x, y, depth);
    }
  }
}

void drawTriangleWireframe(const Varying &v0, const Varying &v1,
                           const Varying &v2,
                           std::vector<uint32_t> &framebuffer, int width,
                           int height) {
  // 转换为整数屏幕坐标
  int x0 = static_cast<int>(v0.screenPos.x);
  int y0 = static_cast<int>(v0.screenPos.y);
  int x1 = static_cast<int>(v1.screenPos.x);
  int y1 = static_cast<int>(v1.screenPos.y);
  int x2 = static_cast<int>(v2.screenPos.x);
  int y2 = static_cast<int>(v2.screenPos.y);

  // 绘制三条边（白色）
  const uint32_t WIREFRAME_COLOR = 0xFFFFFFFF; // 纯白
  draw_line(framebuffer, x0, y0, x1, y1, WIREFRAME_COLOR);
  draw_line(framebuffer, x1, y1, x2, y2, WIREFRAME_COLOR);
  draw_line(framebuffer, x2, y2, x0, y0, WIREFRAME_COLOR);

  // 可选：绘制顶点标记（红色小点）
  put_pixel(framebuffer, x0, y0, 0xFFFF0000);
  put_pixel(framebuffer, x1, y1, 0xFFFF0000);
  put_pixel(framebuffer, x2, y2, 0xFFFF0000);
}

inline bool isFrontFace(const Vector3f &v0_view, const Vector3f &v1_view,
                        const Vector3f &v2_view) {
  Vector3f edge1 = v1_view - v0_view;
  Vector3f edge2 = v2_view - v0_view;
  Vector3f faceNormal = edge1.cross(edge2);
  return faceNormal.z > 0;
}

} // namespace render
} // namespace sr