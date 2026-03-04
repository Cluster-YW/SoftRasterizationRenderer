#pragma once
#include "vector3f.h"

struct Vertex {
  Vector3f position;
  Vector3f color;
  float u, v;

  Vertex()
      : position(Vector3f()), color(Vector3f(1.0f, 1.0f, 1.0f)), u(0.0f),
        v(0.0f) {}
  Vertex(const Vector3f &pos, const Vector3f &col, float u, float v)
      : position(pos), color(col), u(u), v(v) {}
  Vertex(float x, float y, float z, float r, float g, float b, float u, float v)
      : position(x, y, z), color(r, g, b), u(u), v(v) {}
};
