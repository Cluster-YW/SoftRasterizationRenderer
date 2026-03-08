#pragma once

#include "math_utils.h"
#include "shader.h"
#include "texture.h"
#include "vector2f.h"
#include "vector3f.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

// Vertex data for rasterization
struct VertexOut {
  Vector3f screenPos; // x, y: screen_coord, z: NDC Z [-1,1]
  Vector3f viewPos;   // x, y, z: view space position (for clip)
  Vector3f albedo;    // raw_color
  Vector3f light;     // light_color
  Vector3f normal;    // normal (world space)
  Vector2f texcoord;  // texture coordinate
  float invW;         // 1/w

  float zDivW; // z / w
  Vector2f texcoordDivW;
  Vector3f lightDivW;
  Vector3f albedoDivW;
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

inline void drawTriangle(const Varying &v0, const Varying &v1,
                         const Varying &v2, //
                         const ShaderProgram &shader, const Uniforms &uniforms,
                         std::vector<uint32_t> &framebuffer,
                         DepthBuffer &depthBuffer) {

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
  maxX = std::min(maxX, uniforms.screenWidth - 1);
  minY = std::max(minY, 0);
  maxY = std::min(maxY, uniforms.screenHeight - 1);

  // ---- Flat Shading ---
  // I = I_amb + I_diff * max(0, n·l)
  // l is the light direction (normalized)

  // average normal of the triangle
  // Vector3f faceNormal = (v0.normal + v1.normal + v2.normal).normalized();

  // float diff = std::max(0.0f, faceNormal.dot(-lightDir));
  // float amb = 0.2f; // ambient lighting

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
      float z = (v0.screenPos.z * v0.invW * bc.x +  //
                 v1.screenPos.z * v1.invW * bc.y +  //
                 v2.screenPos.z * v2.invW * bc.z) * //
                corrFactor;
      float depth = 0.5f * (z + 1.0f); // NDC Z [-1,1] -> [0,1]

      // if (depth >= depthBuffer.get(x, y)) // early depth test
      //   continue;

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

inline bool isFrontFace(const Vector3f &v0_view, const Vector3f &v1_view,
                        const Vector3f &v2_view) {
  Vector3f edge1 = v1_view - v0_view;
  Vector3f edge2 = v2_view - v0_view;
  Vector3f faceNormal = edge1.cross(edge2);
  return faceNormal.z > 0;
}