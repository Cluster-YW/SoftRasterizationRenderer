#pragma once

#include "math/matrix4x4f.h"
#include "math/vector3f.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

using namespace sr::math;

namespace sr {
namespace geometry {

class Camera {
public:
  Vector3f position;
  Vector3f front;
  Vector3f up;
  Vector3f right;
  Vector3f worldUp;

  float yaw;
  float pitch;

  float movementSpeed;
  float mouseSensitivity;
  float zoom;

  Camera(Vector3f pos = Vector3f(0.0f, 0.0f, 5.0f),
         Vector3f up = Vector3f(0.0f, 1.0f, 0.0f),
         float yaw = -90.0f, // look at -z by default
         float pitch = 0.0f)
      : position(pos), worldUp(up), yaw(yaw), pitch(pitch), movementSpeed(5.0f),
        mouseSensitivity(0.1f), zoom(45.0f) {
    updateCameraVectors();
  }

  Matrix4x4f getViewMatrix() const {
    return Matrix4x4f::lookAt(position, position + front, up);
  }

  void processKeyboard(int direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;

    if (direction == 0) // front
      position += front * velocity;
    if (direction == 1) // back
      position -= front * velocity;
    if (direction == 2) // left
      position -= right * velocity;
    if (direction == 3) // right
      position += right * velocity;
  }

  void processMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // limit pitch
    if (constrainPitch) {
      if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void processMouseScroll(float yoffset) {
    zoom -= yoffset;
    if (zoom < 1.0f)
      zoom = 1.0f;
    if (zoom > 45.0f)
      zoom = 45.0f;
  }

private:
  void updateCameraVectors() {
    Vector3f newFront;
    float yawRad = yaw * M_PI / 180.0f;
    float pitchRad = pitch * M_PI / 180.0f;

    newFront.x = cos(yawRad) * cos(pitchRad);
    newFront.y = sin(pitchRad);
    newFront.z = sin(yawRad) * cos(pitchRad);

    front = newFront.normalized();
    right = front.cross(worldUp).normalized();
    up = right.cross(front).normalized();
  }
};

} // namespace geometry
} // namespace sr