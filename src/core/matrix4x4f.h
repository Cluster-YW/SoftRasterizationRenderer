#pragma once

#include "Vector3f.h"
#include <array>
#include <iostream>

class Matrix4x4f {
public:
  // Data: 16 floats stored in **column-major** order.
  // Elements can be accessed as m[col][row] or m[col * 4 + row].
  std::array<float, 16> m;

  // ---------- Constructors ----------
  // Default constructor: creates identity matrix
  Matrix4x4f();

  // Constructor from 16 floats (col-major order)
  Matrix4x4f(float m00, float m10, float m20, float m30, float m01, float m11,
             float m21, float m31, float m02, float m12, float m22, float m32,
             float m03, float m13, float m23, float m33);

  // Copy constructor
  Matrix4x4f(const Matrix4x4f &other);

  // ---------- Assignment ----------
  Matrix4x4f &operator=(const Matrix4x4f &other);

  // ---------- Element Access ----------
  // Access element at column c (0-3) and row r (0-3)
  float &operator()(int col, int row);
  const float &operator()(int col, int row) const;

  // ---------- Arithmetic Operations ----------
  Matrix4x4f operator*(const Matrix4x4f &rhs) const;

  Vector3f operator*(const Vector3f &v) const;

  Matrix4x4f operator*(float scalar) const;

  Matrix4x4f &operator*=(const Matrix4x4f &rhs);
  Matrix4x4f &operator*=(float scalar);

  // ---------- Comparison ----------
  bool operator==(const Matrix4x4f &other) const;
  bool operator!=(const Matrix4x4f &other) const;

  // ---------- Matrix Operations ----------
  Matrix4x4f transposed() const;

  void transpose();

  float determinant() const;

  Matrix4x4f inverse() const;

  // ---------- Static Transform Matrices ----------
  // Create a translation matrix from a vector
  static Matrix4x4f translation(const Vector3f &t);

  // Create a scaling matrix from a vector
  static Matrix4x4f scaling(const Vector3f &s);

  // Create a rotation matrix around X axis (angle in radians)
  static Matrix4x4f rotationX(float angle);

  // Create a rotation matrix around Y axis
  static Matrix4x4f rotationY(float angle);

  // Create a rotation matrix around Z axis
  static Matrix4x4f rotationZ(float angle);

  // Create an identity matrix
  static Matrix4x4f identity();

  // ---------- Utility ----------
  // Print matrix to stream (for debugging)
  friend std::ostream &operator<<(std::ostream &os, const Matrix4x4f &mat);

  // Static constants (optional)
  static const Matrix4x4f IDENTITY;
  static const Matrix4x4f ZERO;

  // ---------- Generate Special Matrices ----------
  static Matrix4x4f perspective(float fov, float aspect, float near, float far);
  static Matrix4x4f lookAt(const Vector3f &eye, const Vector3f &center,
                           const Vector3f &up);
};