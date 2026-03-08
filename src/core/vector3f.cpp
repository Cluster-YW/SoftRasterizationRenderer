#include "vector3f.h"
#include "vector2f.h"
#include <cmath>
#include <iostream>

// ---------- Constructors ----------
Vector3f::Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
Vector3f::Vector3f(float v) : x(v), y(v), z(v) {}
Vector3f::Vector3f() : x(0.0f), y(0.0f), z(0.0f) {}
Vector3f::Vector3f(const Vector3f &other)
    : x(other.x), y(other.y), z(other.z) {}

// ---------- Assignment Operator ----------
Vector3f &Vector3f::operator=(const Vector3f &other) {
  x = other.x;
  y = other.y;
  z = other.z;
  return *this;
}

// ---------- Arithmetic Operations ----------
Vector3f Vector3f::operator+(const Vector3f &other) const {
  return Vector3f(x + other.x, y + other.y, z + other.z);
}

Vector3f Vector3f::operator-(const Vector3f &other) const {
  return Vector3f(x - other.x, y - other.y, z - other.z);
}

Vector3f Vector3f::operator*(float scalar) const {
  return Vector3f(x * scalar, y * scalar, z * scalar);
}

Vector3f Vector3f::operator/(float scalar) const {
  if (scalar == 0.0f) {
    std::cerr << "Error: Division by zero in Vector3f::operator/()."
              << std::endl;
    return Vector3f(0.0f, 0.0f, 0.0f);
  }
  return Vector3f(x / scalar, y / scalar, z / scalar);
}

Vector3f &Vector3f::operator+=(const Vector3f &other) {
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

Vector3f &Vector3f::operator-=(const Vector3f &other) {
  x -= other.x;
  y -= other.y;
  z -= other.z;
  return *this;
}

Vector3f &Vector3f::operator*=(float scalar) {
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

Vector3f &Vector3f::operator/=(float scalar) {
  x /= scalar;
  y /= scalar;
  z /= scalar;
  return *this;
}

Vector3f Vector3f::operator-() const { return Vector3f(-x, -y, -z); }

// ---------- Comparison ----------
bool Vector3f::operator==(const Vector3f &other) const {
  const float eps = 1e-6f;
  return (std::fabs(x - other.x) < eps) && (std::fabs(y - other.y) < eps) &&
         (std::fabs(z - other.z) < eps);
}

bool Vector3f::operator!=(const Vector3f &other) const {
  return !(*this == other);
}

// ---------- Vector Operations ----------
float Vector3f::dot(const Vector3f &other) const {
  return x * other.x + y * other.y + z * other.z;
}

Vector3f Vector3f::cross(const Vector3f &other) const {
  return Vector3f(y * other.z - z * other.y, z * other.x - x * other.z,
                  x * other.y - y * other.x);
}

// product by element
Vector3f Vector3f::product(const Vector3f &other) const {
  return Vector3f(x * other.x, y * other.y, z * other.z);
}

float Vector3f::length() const { return sqrtf(x * x + y * y + z * z); }

float Vector3f::lengthSquared() const { return x * x + y * y + z * z; }

Vector3f Vector3f::normalized() const {
  float len = length();
  if (len > 0.0f)
    return Vector3f(x / len, y / len, z / len);
  else
    return Vector3f(0.0f, 0.0f, 0.0f);
}

void Vector3f::normalize() {
  float len = length();
  if (len > 0.0f) {
    x /= len;
    y /= len;
    z /= len;
  } else
    x = y = z = 0.0f;
}

// ---------- Utility ----------
Vector3f Vector3f::clamp(const Vector3f &min, const Vector3f &max) const {
  return Vector3f(std::max(min.x, std::min(max.x, x)),
                  std::max(min.y, std::min(max.y, y)),
                  std::max(min.z, std::min(max.z, z)));
}
Vector3f Vector3f::clamp(float min, float max) const {
  return Vector3f(std::max(min, std::min(max, x)),
                  std::max(min, std::min(max, y)),
                  std::max(min, std::min(max, z)));
}

const Vector3f Vector3f::ZERO(0.0f, 0.0f, 0.0f);
const Vector3f Vector3f::X_AXIS(1.0f, 0.0f, 0.0f);
const Vector3f Vector3f::Y_AXIS(0.0f, 1.0f, 0.0f);
const Vector3f Vector3f::Z_AXIS(0.0f, 0.0f, 1.0f);

std::ostream &operator<<(std::ostream &os, const Vector3f &v) {
  os << "v3f(" << v.x << ", " << v.y << ", " << v.z << ")";
  return os;
}