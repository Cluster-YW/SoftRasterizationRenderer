#pragma once
#include "vector3f.h"

struct Vertex {
  Vector3f position;
  Vector3f color;
  Vector3f normal;
  float u, v;

  // default constructor
  Vertex()
      : position(Vector3f()), color(Vector3f(1.0f, 1.0f, 1.0f)),
        normal(Vector3f(0, 0, 1)), u(0), v(0) {}

  // constructor with parameters
  Vertex(const Vector3f &pos, const Vector3f &col, const Vector3f &nrm, float u,
         float v)
      : position(pos), color(col), normal(nrm), u(u), v(v) {}
};
