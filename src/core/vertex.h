#pragma once
#include "vector2f.h"
#include "vector3f.h"

struct Vertex {
  Vector3f position;
  Vector3f color;
  Vector3f normal;
  Vector2f texcoord;

  // default constructor
  Vertex() : position(), color(1, 1, 1), normal(0, 0, 1), texcoord(0, 0) {}

  // constructor with parameters
  Vertex(const Vector3f &pos, const Vector3f &nrm, const Vector2f &uv)
      : position(pos), normal(nrm), color(1, 1, 1), texcoord(uv) {}
};
