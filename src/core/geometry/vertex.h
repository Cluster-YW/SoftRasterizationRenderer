#pragma once
#include "math/vector2f.h"
#include "math/vector3f.h"

namespace sr {
namespace geometry {

struct Vertex {
  math::Vector3f position;
  math::Vector3f color;
  math::Vector3f normal;
  math::Vector2f texcoord;

  // default constructor
  Vertex() : position(), color(1, 1, 1), normal(0, 0, 1), texcoord(0, 0) {}

  // constructor with parameters
  Vertex(const math::Vector3f &pos, const math::Vector3f &nrm,
         const math::Vector2f &uv)
      : position(pos), normal(nrm), color(1, 1, 1), texcoord(uv) {}
};

} // namespace geometry
} // namespace sr