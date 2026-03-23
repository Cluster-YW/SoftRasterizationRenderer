#pragma once

#include <cmath>
#include <iostream>

namespace sr {
namespace math {

class Vector3f {
public:
  // Data members: three float components
  float x, y, z;

  // ---------- Constructors ----------
  // Default constructor: initializes to (0, 0, 0)
  Vector3f();

  // Constructor with same values
  Vector3f(float v);

  // Constructor with three values
  Vector3f(float x, float y, float z);

  // Copy constructor
  Vector3f(const Vector3f &other);

  // ---------- Assignment Operator ----------
  Vector3f &operator=(const Vector3f &other);

  // ---------- Arithmetic Operations ----------
  Vector3f operator+(const Vector3f &rhs) const;
  Vector3f operator-(const Vector3f &rhs) const;
  Vector3f operator*(float scalar) const;
  Vector3f operator/(float scalar) const;

  // Compound assignment operators
  Vector3f &operator+=(const Vector3f &rhs);
  Vector3f &operator-=(const Vector3f &rhs);
  Vector3f &operator*=(float scalar);
  Vector3f &operator/=(float scalar);

  // Unary minus: return a new vector with all components negated
  Vector3f operator-() const;

  // ---------- Comparison ----------
  // Equality with tolerance (optional, useful for floating point)
  bool operator==(const Vector3f &rhs) const;
  bool operator!=(const Vector3f &rhs) const;

  // ---------- Vector Operations ----------
  float dot(const Vector3f &rhs) const;
  Vector3f cross(const Vector3f &rhs) const;
  Vector3f product(const Vector3f &rhs) const;
  float length() const;
  float lengthSquared() const;
  Vector3f normalized() const;
  void normalize();

  // ---------- Utility ----------
  Vector3f clamp(const Vector3f &min, const Vector3f &max) const;
  Vector3f clamp(float min, float max) const;

  // Interpolation between three vectors
  template <typename T>
  T interpolate(const T &a, const T &b, const T &c) const {
    return a * x + b * y + c * z;
  }

  // Static constants
  static const Vector3f ZERO;
  static const Vector3f X_AXIS;
  static const Vector3f Y_AXIS;
  static const Vector3f Z_AXIS;

  friend std::ostream &operator<<(std::ostream &os, const Vector3f &v);
};

// ---------- Scalar multiplication from left side ----------
inline Vector3f operator*(float scalar, const Vector3f &v) {
  return v * scalar;
}

} // namespace math
} // namespace sr