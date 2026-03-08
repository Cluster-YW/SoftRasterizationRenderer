#pragma once

#include "matrix4x4f.h"
#include "texture.h"
#include "vector2f.h"
#include "vector3f.h"
#include "vertex.h"
#include <functional>

struct Uniforms {
  Matrix4x4f model;
  Matrix4x4f view;
  Matrix4x4f proj;
  Matrix4x4f normalMat;
  Vector3f lightDir;
  float ambient;
  Texture *texture;
  bool useTexture;
  int screenWidth;
  int screenHeight;

  Matrix4x4f mvp() const { return proj * view * model; }
};

// Output of vertex shader
// (Data to be interpolated)
struct Varying {
  Vector3f position;
  Vector3f normal;
  Vector3f screenPos; // Screen space position
  Vector2f texcoord;
  Vector3f color;
  float invW;
};

class ShaderProgram {
public:
  virtual ~ShaderProgram() = default;

  using VertexShader = std::function<Varying(const Vertex &, const Uniforms &)>;
  using FragmentShader =
      std::function<Vector3f(const Varying &, const Uniforms &)>;

  VertexShader vertexShader;
  FragmentShader fragmentShader;

  ShaderProgram(VertexShader vs, FragmentShader fs)
      : vertexShader(vs), fragmentShader(fs) {}
};

namespace Shaders {
// Default shader: lambert shading
inline ShaderProgram CreateLambertShader() {
  auto vs = [](const Vertex &v, const Uniforms &u) -> Varying {
    Varying out;

    // Model transform
    Vector3f world = u.model * v.position;
    Vector3f worldNormal = u.normalMat * v.normal;
    worldNormal.normalize();

    // Lighting on vertex
    float diff = std::max(0.0f, worldNormal.dot(u.lightDir));
    out.color = Vector3f(u.ambient + diff); // Light Intensity
    out.position = world;
    out.normal = worldNormal;
    out.texcoord = v.texcoord;

    Vector3f ndc = u.mvp() * v.position;

    // Perspective divide
    out.invW = 1.0f / (-(u.view * world).z);

    out.screenPos.x = (ndc.x + 1.0f) * 0.5f * u.screenWidth;
    out.screenPos.y = (1.0f - ndc.y) * 0.5f * u.screenHeight;
    out.screenPos.z = ndc.z;

    return out;
  };

  auto fs = [](const Varying &f, const Uniforms &u) -> Vector3f {
    Vector3f albedo(1.0f, 1.0f, 1.0f);
    if (u.useTexture && u.texture != nullptr) {
      albedo = u.texture->sampleNearest(f.texcoord.x, f.texcoord.y);
    }

    return albedo.product(f.color);
  };

  return ShaderProgram(vs, fs);
}

}; // namespace Shaders