#pragma once

#include <cmath>

struct Vector2f {
  float x, y;

  Vector2f() : x(0), y(0) {}
  Vector2f(float x, float y) : x(x), y(y) {}

  Vector2f operator+(const Vector2f &o) const {
    return Vector2f(x + o.x, y + o.y);
  }
  Vector2f operator-(const Vector2f &o) const {
    return Vector2f(x - o.x, y - o.y);
  }
  Vector2f operator*(float s) const { return Vector2f(x * s, y * s); }
};

inline Vector2f operator*(float s, const Vector2f &v) { return v * s; }