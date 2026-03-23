#pragma once

#include "geometry/vertex.h"
#include "math/matrix4x4f.h"
#include "math/vector2f.h"
#include "math/vector3f.h"
#include "resource/texture.h"
#include <algorithm>
#include <functional>

namespace sr {
namespace render {

using Vec3 = math::Vector3f;
using Vec2 = math::Vector2f;
using Mat4 = math::Matrix4x4f;
using Vertex = geometry::Vertex;

struct Uniforms {
  Mat4 model;
  Mat4 view;
  Mat4 proj;
  Mat4 normalMat;

  Vec3 lightDir;
  Vec3 lightColor;
  Vec3 cameraPos;

  float ambient;
  float shininess;
  Vec3 specularColor;

  Texture *texture;
  bool useTexture;

  int screenWidth;
  int screenHeight;

  Mat4 mvp() const { return proj * view * model; }
};

// Output of vertex shader
// (Data to be interpolated)
struct Varying {
  Vec3 position;
  Vec3 normal;
  Vec3 screenPos; // Screen space position
  Vec3 viewPos;   // View space position (for culling)
  Vec2 texcoord;
  Vec3 color;
  float invW;
};

class ShaderProgram {
public:
  virtual ~ShaderProgram() = default;

  using VertexShader = std::function<Varying(const Vertex &, const Uniforms &)>;
  using FragmentShader = std::function<Vec3(const Varying &, const Uniforms &)>;

  VertexShader vertexShader;
  FragmentShader fragmentShader;

  ShaderProgram(VertexShader vs, FragmentShader fs)
      : vertexShader(vs), fragmentShader(fs) {}
};
namespace Shaders {

using Vec3 = math::Vector3f;
using Vec2 = math::Vector2f;
using Mat4 = math::Matrix4x4f;
using Vertex = geometry::Vertex;

auto general_vs = [](const Vertex &v, const Uniforms &u) -> Varying {
  Varying out;

  // Model transform
  Vec3 world = u.model * v.position;
  Vec3 worldNormal = u.normalMat * v.normal;
  worldNormal.normalize();

  // View transform
  Vec3 viewPos = u.view * world;
  out.viewPos = viewPos;
  out.invW = 1.0f / (-viewPos.z);

  // Lighting on vertex
  float diff = std::max(0.0f, worldNormal.dot(-u.lightDir));
  out.color = Vec3(u.ambient + diff); // Light Intensity
  out.position = world;
  out.normal = worldNormal;
  out.texcoord = v.texcoord;

  // MVP transform
  Vec3 ndc = u.mvp() * v.position;

  out.screenPos.x = (ndc.x + 1.0f) * 0.5f * u.screenWidth;
  out.screenPos.y = (1.0f - ndc.y) * 0.5f * u.screenHeight;
  out.screenPos.z = ndc.z;

  return out;
};

// Default shader: lambert shading
inline ShaderProgram CreateLambertShader() {

  auto fs = [](const Varying &f, const Uniforms &u) -> Vec3 {
    Vec3 albedo(1.0f, 1.0f, 1.0f);
    if (u.useTexture && u.texture != nullptr) {
      albedo = u.texture->sampleNearest(f.texcoord.x, f.texcoord.y);
    }

    return albedo.product(f.color);
  };

  return ShaderProgram(general_vs, fs);
}

inline ShaderProgram CreateBlinnPhongShader() {
  auto fs = [](const Varying &f, const Uniforms &u) -> Vec3 {
    // get albedo (base color)
    Vec3 albedo = f.color;
    if (u.useTexture && u.texture != nullptr) {
      albedo = u.texture->sampleNearest(f.texcoord.x, f.texcoord.y);
    }

    // prepare normal and lighting vectors
    Vec3 N = f.normal.normalized();    // normal vector
    Vec3 L = -u.lightDir.normalized(); // light direction (towards light source)
    Vec3 V = (u.cameraPos - f.viewPos)
                 .normalized(); // view direction (towards camera)

    // diffuse term (lambert shading)
    float NdotL = std::max(N.dot(L), 0.0f);
    Vec3 diffuse = albedo * (u.ambient + NdotL);

    // specular term (phong shading)
    Vec3 H = (L + V).normalized(); // halfway vector
    float NdotH = std::max(N.dot(H), 0.0f);
    float spec = pow(NdotH, u.shininess);

    if (NdotL > 0.0f)
      spec = std::pow(NdotH, u.shininess);
    else
      spec = 0.0f;

    Vec3 specular = u.specularColor * spec;

    // mix diffuse and specular terms
    return (diffuse + specular).clamp(0.0f, 1.0f);
  };

  return ShaderProgram(general_vs, fs);
}
}; // namespace Shaders
} // namespace render

} // namespace sr