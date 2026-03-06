#include "Matrix4x4f.h"
#include "matrix4x4f.h"
#include <cassert>
#include <cmath>

// ---------- Constructors ----------
Matrix4x4f::Matrix4x4f() {
  m.fill(0.0f);
  m[0] = m[5] = m[10] = m[15] = 1.0f;
}

Matrix4x4f::Matrix4x4f(float m00, float m10, float m20, float m30, //
                       float m01, float m11, float m21, float m31, //
                       float m02, float m12, float m22, float m32, //
                       float m03, float m13, float m23, float m33) {
  // 列主序填充
  m[0] = m00;
  m[1] = m10;
  m[2] = m20;
  m[3] = m30;
  m[4] = m01;
  m[5] = m11;
  m[6] = m21;
  m[7] = m31;
  m[8] = m02;
  m[9] = m12;
  m[10] = m22;
  m[11] = m32;
  m[12] = m03;
  m[13] = m13;
  m[14] = m23;
  m[15] = m33;
}

Matrix4x4f::Matrix4x4f(const Matrix4x4f &other) : m(other.m) {}

// ---------- Assignment ----------
Matrix4x4f &Matrix4x4f::operator=(const Matrix4x4f &other) {
  m = other.m;
  return *this;
}

// ---------- Element Access ----------
float &Matrix4x4f::operator()(int col, int row) { return m[col * 4 + row]; }

const float &Matrix4x4f::operator()(int col, int row) const {
  return m[col * 4 + row];
}

// ---------- Arithmetic Operations ----------
Matrix4x4f Matrix4x4f::operator*(const Matrix4x4f &rhs) const {
  Matrix4x4f res;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      float sum = 0.0f;
      for (int k = 0; k < 4; ++k) {
        sum += (*this)(k, j) * rhs(i, k);
      }
      res(i, j) = sum;
    }
  }
  return res;
}

// 矩阵乘以标量
Matrix4x4f Matrix4x4f::operator*(float scalar) const {
  Matrix4x4f res;
  for (int i = 0; i < 16; ++i) {
    res.m[i] = m[i] * scalar;
  }
  return res;
}

Vector3f Matrix4x4f::operator*(const Vector3f &v) const {
  float x = v.x, y = v.y, z = v.z;
  float w = 1.0f;
  float xp = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
  float yp = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
  float zp = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
  float wp = m[3] * x + m[7] * y + m[11] * z + m[15] * w;

  // keep w=1.0f
  if (wp != 0.0f && wp != 1.0f) {
    return Vector3f(xp / wp, yp / wp, zp / wp);
  } else {
    return Vector3f(xp, yp, zp);
  }
}

Matrix4x4f &Matrix4x4f::operator*=(const Matrix4x4f &rhs) {
  *this = *this * rhs;
  return *this;
}

Matrix4x4f &Matrix4x4f::operator*=(float scalar) {
  for (int i = 0; i < 16; ++i) {
    m[i] *= scalar;
  }
  return *this;
}

// ---------- Comparison ----------
bool Matrix4x4f::operator==(const Matrix4x4f &other) const {
  const float eps = 1e-6f;
  for (int i = 0; i < 16; ++i) {
    if (std::fabs(m[i] - other.m[i]) >= eps)
      return false;
  }
  return true;
}

bool Matrix4x4f::operator!=(const Matrix4x4f &other) const {
  return !(*this == other);
}

// ---------- Matrix Operations ----------
Matrix4x4f Matrix4x4f::transposed() const {
  Matrix4x4f res;
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      res(col, row) = (*this)(row, col);
    }
  }
  return res;
}

void Matrix4x4f::transpose() { *this = this->transposed(); }

float Matrix4x4f::determinant() const {
  // Laplace expansion with row 0
  const float a11 = (*this)(0, 0), a12 = (*this)(1, 0), a13 = (*this)(2, 0),
              a14 = (*this)(3, 0);
  const float a21 = (*this)(0, 1), a22 = (*this)(1, 1), a23 = (*this)(2, 1),
              a24 = (*this)(3, 1);
  const float a31 = (*this)(0, 2), a32 = (*this)(1, 2), a33 = (*this)(2, 2),
              a34 = (*this)(3, 2);
  const float a41 = (*this)(0, 3), a42 = (*this)(1, 3), a43 = (*this)(2, 3),
              a44 = (*this)(3, 3);

  float det =
      a11 * (a22 * (a33 * a44 - a34 * a43) - a23 * (a32 * a44 - a34 * a42) +
             a24 * (a32 * a43 - a33 * a42)) -
      a12 * (a21 * (a33 * a44 - a34 * a43) - a23 * (a31 * a44 - a34 * a41) +
             a24 * (a31 * a43 - a33 * a41)) +
      a13 * (a21 * (a32 * a44 - a34 * a42) - a22 * (a31 * a44 - a34 * a41) +
             a24 * (a31 * a42 - a32 * a41)) -
      a14 * (a21 * (a32 * a43 - a33 * a42) - a22 * (a31 * a43 - a33 * a41) +
             a23 * (a31 * a42 - a32 * a41));
  return det;
}

static float cofactor(const Matrix4x4f &mat, int col, int row) {
  float sub[3][3];
  int si = 0, sj = 0;
  for (int i = 0; i < 4; ++i) {
    if (i == col)
      continue;
    sj = 0;
    for (int j = 0; j < 4; ++j) {
      if (j == row)
        continue;
      sub[si][sj] = mat(i, j);
      ++sj;
    }
    ++si;
  }
  // calculate determinant of 3x3 submatrix
  float det3 = sub[0][0] * (sub[1][1] * sub[2][2] - sub[1][2] * sub[2][1]) -
               sub[0][1] * (sub[1][0] * sub[2][2] - sub[1][2] * sub[2][0]) +
               sub[0][2] * (sub[1][0] * sub[2][1] - sub[1][1] * sub[2][0]);

  return ((col + row) % 2 == 0) ? det3 : -det3;
}

Matrix4x4f Matrix4x4f::inverse() const {
  float det = determinant();
  if (std::fabs(det) < 1e-12f) {
    // uninvertible matrix
    assert(false && "Matrix is singular, cannot invert.");
    return identity();
  }

  Matrix4x4f adj; // adjugate matrix
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      adj(row, col) = cofactor(*this, col, row); // adj = transpose(cofactor)
    }
  }
  // A^-1 = A* / |A|
  return adj * (1.0f / det);
}

// ---------- Static Transform Matrices ----------
Matrix4x4f Matrix4x4f::identity() { return Matrix4x4f(); }

Matrix4x4f Matrix4x4f::translation(const Vector3f &t) {
  Matrix4x4f mat;
  mat(3, 0) = t.x;
  mat(3, 1) = t.y;
  mat(3, 2) = t.z;
  return mat;
}

Matrix4x4f Matrix4x4f::scaling(const Vector3f &s) {
  Matrix4x4f mat;
  mat(0, 0) = s.x;
  mat(1, 1) = s.y;
  mat(2, 2) = s.z;
  return mat;
}

Matrix4x4f Matrix4x4f::rotationX(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  Matrix4x4f mat;
  mat(1, 1) = c;
  mat(2, 1) = -s;
  mat(1, 2) = s;
  mat(2, 2) = c;
  return mat;
}

Matrix4x4f Matrix4x4f::rotationY(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  Matrix4x4f mat;
  mat(2, 2) = c;
  mat(0, 2) = -s;
  mat(2, 0) = s;
  mat(0, 0) = c;
  return mat;
}

Matrix4x4f Matrix4x4f::rotationZ(float angle) {
  float c = cosf(angle);
  float s = sinf(angle);
  Matrix4x4f mat;
  mat(0, 0) = c;
  mat(1, 0) = -s;
  mat(0, 1) = s;
  mat(1, 1) = c;
  return mat;
}

// ---------- Utility ----------
std::ostream &operator<<(std::ostream &os, const Matrix4x4f &mat) {
  os << "Matrix4x4f[\n";
  for (int row = 0; row < 4; ++row) {
    os << "  ";
    for (int col = 0; col < 4; ++col) {
      os << mat(col, row) << " ";
    }
    os << "\n";
  }
  os << "]";
  return os;
}

// ---------- Utility ----------
const Matrix4x4f Matrix4x4f::IDENTITY;
const Matrix4x4f Matrix4x4f::ZERO([] {
  Matrix4x4f m;
  m.m.fill(0.0f);
  return m;
}());

Matrix4x4f Matrix4x4f::perspective(float fov, float aspect, float near,
                                   float far) {
  float tanHalfFov = tanf(fov / 2.0f);
  Matrix4x4f mat = Matrix4x4f::ZERO;
  /*
  [ 1/asp*tan(Fov/2), 0,            0,            0
    0,                1/tan(Fov/2), 0,            0
    0,                0,            -(f+n)/(f-n), -1
    0,                0,            -2*f*n/(f-n), 0 ]
  */
  mat(0, 0) = 1.0f / (aspect * tanHalfFov);
  mat(1, 1) = 1.0f / (tanHalfFov);
  mat(2, 2) = -(far + near) / (far - near);
  mat(2, 3) = -1.0f;
  mat(3, 2) = -2.0f * far * near / (far - near);
  return mat;
}

Matrix4x4f Matrix4x4f::lookAt(const Vector3f &eye, const Vector3f &center,
                              const Vector3f &up) {
  // Base vectors of camera coordinate system
  Vector3f f = (center - eye).normalized(); // fronthand vector(-z)
  Vector3f s = f.cross(up).normalized();    // righthand vector(-x)
  Vector3f u = s.cross(f);                  // up vector(y)

  // Rotate
  Matrix4x4f rot;
  rot(0, 0) = s.x;
  rot(1, 0) = s.y;
  rot(2, 0) = s.z;
  rot(3, 0) = 0.0f;
  rot(0, 1) = u.x;
  rot(1, 1) = u.y;
  rot(2, 1) = u.z;
  rot(3, 1) = 0.0f;
  rot(0, 2) = -f.x;
  rot(1, 2) = -f.y;
  rot(2, 2) = -f.z;
  rot(3, 2) = 0.0f;
  rot(3, 3) = 1.0f;

  // Translation
  Matrix4x4f trans = Matrix4x4f::translation(-eye);

  // Final matrix
  return rot * trans;
}

Matrix4x4f Matrix4x4f::normalMatrix() const {
  Matrix4x4f inv = this->inverse();
  return inv.transposed();
}