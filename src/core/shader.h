#pragma once

#include "matrix4x4f.h"
#include "texture.h"
#include "vector2f.h"
#include "vector3f.h"
#include "vertex.h"
#include <algorithm>
#include <functional>

struct Uniforms {
  Matrix4x4f model;
  Matrix4x4f view;
  Matrix4x4f proj;
  Matrix4x4f normalMat;

  Vector3f lightDir;
  Vector3f lightColor;
  Vector3f cameraPos;

  float ambient;
  float shininess;
  Vector3f specularColor;

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
  Vector3f viewPos;   // View space position (for culling)
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

auto general_vs = [](const Vertex &v, const Uniforms &u) -> Varying {
  Varying out;

  // Model transform
  Vector3f world = u.model * v.position;
  Vector3f worldNormal = u.normalMat * v.normal;
  worldNormal.normalize();

  // View transform
  Vector3f viewPos = u.view * world;
  out.viewPos = viewPos;
  out.invW = 1.0f / (-viewPos.z);

  // Lighting on vertex
  float diff = std::max(0.0f, worldNormal.dot(-u.lightDir));
  out.color = Vector3f(u.ambient + diff); // Light Intensity
  out.position = world;
  out.normal = worldNormal;
  out.texcoord = v.texcoord;

  // MVP transform
  Vector3f ndc = u.mvp() * v.position;

  out.screenPos.x = (ndc.x + 1.0f) * 0.5f * u.screenWidth;
  out.screenPos.y = (1.0f - ndc.y) * 0.5f * u.screenHeight;
  out.screenPos.z = ndc.z;

  return out;
};

// Default shader: lambert shading
inline ShaderProgram CreateLambertShader() {

  auto fs = [](const Varying &f, const Uniforms &u) -> Vector3f {
    Vector3f albedo(1.0f, 1.0f, 1.0f);
    if (u.useTexture && u.texture != nullptr) {
      albedo = u.texture->sampleNearest(f.texcoord.x, f.texcoord.y);
    }

    return albedo.product(f.color);
  };

  return ShaderProgram(general_vs, fs);
}

inline ShaderProgram CreateBlinnPhongShader() {
  auto fs = [](const Varying &f, const Uniforms &u) -> Vector3f {
    // get albedo (base color)
    Vector3f albedo = f.color;
    if (u.useTexture && u.texture != nullptr) {
      albedo = u.texture->sampleNearest(f.texcoord.x, f.texcoord.y);
    }

    // prepare normal and lighting vectors
    Vector3f N = f.normal.normalized(); // normal vector
    Vector3f L =
        -u.lightDir.normalized(); // light direction (towards light source)
    Vector3f V = (u.cameraPos - f.viewPos)
                     .normalized(); // view direction (towards camera)

    // diffuse term (lambert shading)
    float NdotL = std::max(N.dot(L), 0.0f);
    Vector3f diffuse = albedo * (u.ambient + NdotL);

    // specular term (phong shading)
    Vector3f H = (L + V).normalized(); // halfway vector
    float NdotH = std::max(N.dot(H), 0.0f);
    float spec = pow(NdotH, u.shininess);

    if (NdotL > 0.0f)
      spec = std::pow(NdotH, u.shininess);
    else
      spec = 0.0f;

    Vector3f specular = u.specularColor * spec;

    // mix diffuse and specular terms
    return (diffuse + specular).clamp(0.0f, 1.0f);
  };

  return ShaderProgram(general_vs, fs);
}
}; // namespace Shaders