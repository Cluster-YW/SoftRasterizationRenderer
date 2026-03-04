#pragma once
#include "vector3f.h"

struct Vertex {
  Vector3f position;
  Vector3f color;

  Vertex() : position(Vector3f()), color(Vector3f(1.0f, 1.0f, 1.0f)) {}
  Vertex(const Vector3f &pos, const Vector3f &col)
      : position(pos), color(col) {}
  Vertex(float x, float y, float z, float r, float g, float b)
      : position(x, y, z), color(r, g, b) {}
};
