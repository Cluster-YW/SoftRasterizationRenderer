#pragma once

#include <cstdint>
#include <vector>

namespace sr {
namespace render {

class DepthBuffer {
public:
  int width, height;
  std::vector<float> buffer;

  DepthBuffer(int width, int height) : width(width), height(height) {
    buffer.resize(width * height);
    clear();
  }

  void clear() { std::fill(buffer.begin(), buffer.end(), 1.0f); }

  float get(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height)
      return 1.0f;
    return buffer[y * width + x];
  }
  void set(int x, int y, float z) {
    if (x < 0 || x >= width || y < 0 || y >= height)
      return;
    buffer[y * width + x] = z;
  }
};

class Framebuffer {
public:
  Framebuffer(int width, int height)
      : width_(width), height_(height),
        color_buffer_(width * height, 0xFF000000), // 默认黑色不透明
        depth_buffer_(width, height) {}

  // 核心操作
  void clear(uint32_t color = 0xFF000000) {
    std::fill(color_buffer_.begin(), color_buffer_.end(), color);
    depth_buffer_.clear();
  }

  void put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_)
      return;
    color_buffer_[y * width_ + x] = color;
  }

  uint32_t get_pixel(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_)
      return 0;
    return color_buffer_[y * width_ + x];
  }

  // 数据访问（供 SDL 使用）
  uint32_t *data() { return color_buffer_.data(); }
  const uint32_t *data() const { return color_buffer_.data(); }
  DepthBuffer &depth() { return depth_buffer_; }

  int width() const { return width_; }
  int height() const { return height_; }
  size_t size() const { return color_buffer_.size(); }

private:
  int width_, height_;
  std::vector<uint32_t> color_buffer_;
  DepthBuffer depth_buffer_;
};
} // namespace render
} // namespace sr