#pragma once
#include "math/vector2f.h"
#include "math/vector3f.h"

#include "stb/stb_image.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

using namespace sr::math;

class Texture {
public:
  int width, height, channels;
  std::vector<uint8_t> data; // 按RGBA顺序连续存储

  Texture() : width(0), height(0) {}

  bool loadFromFile(const char *filename) {
    unsigned char *img = stbi_load(filename, &width, &height, &channels, 4);
    if (!img) {
      std::cerr << "Failed to load texture: " << filename << std::endl;
      std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
      return false;
    }
    std::cout << "Loaded texture: " << filename << " (" << width << "x"
              << height << ", channels: " << channels << ")" << std::endl;

    // duplicate the image data into a vector
    // image start at left-top corner, but we need to start at left-bottom
    // corner
    data.resize(width * height * 4);
    for (int y = 0; y < height; ++y) {
      int srcRow = y;
      int dstRow = height - 1 - y;
      for (int x = 0; x < width; ++x) {
        int srcIdx = (srcRow * width + x) * 4;
        int dstIdx = (dstRow * width + x) * 4;
        data[dstIdx + 0] = img[srcIdx + 0]; // R
        data[dstIdx + 1] = img[srcIdx + 1]; // G
        data[dstIdx + 2] = img[srcIdx + 2]; // B
        data[dstIdx + 3] = img[srcIdx + 3]; // A
      }
    }

    stbi_image_free(img);
    return true;
  };

  void createCheckerboard(int w, int h, int checkSize = 32) {
    width = w;
    height = h;
    data.resize(w * h * 4);

    for (int y = 0; y < h; ++y) {
      for (int x = 0; x < w; ++x) {
        bool check = ((x / checkSize) + (y / checkSize)) % 2 == 0;
        uint8_t c = check ? 255 : 50; // 白/深灰
        int idx = (y * w + x) * 4;
        data[idx + 0] = c;   // R
        data[idx + 1] = c;   // G
        data[idx + 2] = c;   // B
        data[idx + 3] = 255; // A
      }
    }
  }

  // use bilinear interpolation for default texture sampling
  Vector3f sample(float u, float v) const {
    // repeat texture if outside of range
    u = u - std::floor(u);
    v = v - std::floor(v);

    // transform texture coordinates to pixel coordinates
    float x = u * (width - 1);
    float y = v * (height - 1);

    // get 4 pixels around the pixel coordinates
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int x1 = std::min(x0 + 1, width - 1);
    int y1 = std::min(y0 + 1, height - 1);

    // interpolation factors
    float fx = x - x0;
    float fy = y - y0;

    auto getPixel = [&](int px, int py) -> Vector3f {
      int idx = (py * width + px) * 4;
      return Vector3f(data[idx] / 255.0f, data[idx + 1] / 255.0f,
                      data[idx + 2] / 255.0f);
    };

    Vector3f c00 = getPixel(x0, y0);
    Vector3f c10 = getPixel(x1, y0);
    Vector3f c01 = getPixel(x0, y1);
    Vector3f c11 = getPixel(x1, y1);

    // bilinear interpolation
    Vector3f c0 = c00 * (1 - fx) + c10 * fx;
    Vector3f c1 = c01 * (1 - fx) + c11 * fx;
    return c0 * (1 - fy) + c1 * fy;
  }

  Vector3f sampleNearest(float u, float v) const {
    u = u - std::floor(u);
    v = v - std::floor(v);
    int x = static_cast<int>(u * width) % width;
    int y = static_cast<int>(v * height) % height;
    int idx = (y * width + x) * 4;
    return Vector3f(data[idx] / 255.0f, data[idx + 1] / 255.0f,
                    data[idx + 2] / 255.0f);
  }
};