#include "core/matrix4x4f.h"
#include "core/rasterizer.h"
#include "core/vector3f.h"
#include "core/vertex.h"
#include "viewport.h"
#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

using Framebuffer = std::vector<uint32_t>;

void put_pixel(Framebuffer &framebuffer, int x, int y, uint32_t color) {
  framebuffer[y * SCREEN_WIDTH + x] = color;
}

struct VertexData {
  Vector3f screen;
  Vector3f color;
  float u, v;
};

void draw_line(Framebuffer &framebuffer, int x0, int y0, int x1, int y1,
               uint32_t color) {
  bool steep =
      std::abs(y1 - y0) > std::abs(x1 - x0); // determine if the line is steep
  if (steep) { // if the line is steep, swap the x and y coordinates
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  // ensure x0 <= x1
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // Bresenham's algorithm
  int dx = x1 - x0;
  int dy = std::abs(y1 - y0);
  int err = dx / 2;
  int ystep = (y0 < y1) ? 1 : -1;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      put_pixel(framebuffer, y, x, color);
    } else {
      put_pixel(framebuffer, x, y, color);
    }
    err -= dy;
    if (err < 0) {
      y += ystep;
      err += dx;
    }
  }
}

int main(int argc, char *argv[]) {
  // ********** SDL initialization **********
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow(
      "SoftRasterizationRenderer", SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           SCREEN_WIDTH, SCREEN_HEIGHT);
  if (texture == nullptr) {
    std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  // ********** SoftRasterizationRenderer initialization **********

  Framebuffer framebuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0x00000000);
  DepthBuffer depthBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

  bool quit = false;
  SDL_Event event;

  const uint32_t RED = 0xFFFF0000;
  const uint32_t GREEN = 0xFF00FF00;
  const uint32_t BLUE = 0xFF0000FF;

  std::vector<Vertex> cubeVertices = {
      // (z = -1)
      Vertex(Vector3f(-1, -1, -1), Vector3f(1, 0, 0), 0.0f, 0.0f), // 0
      Vertex(Vector3f(1, -1, -1), Vector3f(0, 1, 0), 1.0f, 0.0f),  // 1
      Vertex(Vector3f(1, 1, -1), Vector3f(0, 0, 1), 1.0f, 1.0f),   // 2
      Vertex(Vector3f(-1, 1, -1), Vector3f(1, 1, 0), 0.0f, 1.0f),  // 3

      // (z = 1)
      Vertex(Vector3f(-1, -1, 1), Vector3f(1, 0, 1), 0.0f, 0.0f),     // 4
      Vertex(Vector3f(1, -1, 1), Vector3f(0, 1, 1), 1.0f, 0.0f),      // 5
      Vertex(Vector3f(1, 1, 1), Vector3f(1, 1, 1), 1.0f, 1.0f),       // 6
      Vertex(Vector3f(-1, 1, 1), Vector3f(0.5, 0.5, 0.5), 0.0f, 1.0f) // 7
  };

  std::vector<std::array<int, 3>> cubeTriangles = {
      {0, 2, 1}, {0, 3, 2}, {4, 5, 6}, {4, 6, 7}, {0, 4, 7}, {0, 7, 3},
      {1, 2, 6}, {1, 6, 5}, {0, 1, 5}, {0, 5, 4}, {3, 7, 6}, {3, 6, 2}};

  Matrix4x4f modelMatrix = Matrix4x4f::identity();
  Matrix4x4f viewMatrix = Matrix4x4f::lookAt(
      Vector3f(3, 2, 5), Vector3f(0, 0, 0), Vector3f(0, 1, 0));
  Matrix4x4f projMatrix = Matrix4x4f::perspective(
      45.0f * M_PI / 180.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);

  float angle = 0.0f;

  // ********** Main loop **********
  while (!quit) {

    // event handling
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }

    // update model matrix
    angle += 0.01f;
    Matrix4x4f modelMatrix =
        Matrix4x4f::rotationY(angle) * Matrix4x4f::rotationX(angle * 0.5f);

    // === rendering ===
    std::fill(framebuffer.begin(), framebuffer.end(),
              0x00000000); // clear framebuffer
    depthBuffer.clear();   // clear depth buffer

    std::vector<VertexData> transformedData(cubeVertices.size());

    for (size_t i = 0; i < cubeVertices.size(); i++) {
      // MVP
      Vector3f world = modelMatrix * cubeVertices[i].position;
      Vector3f view = viewMatrix * world;
      Vector3f clip = projMatrix * view; // Vector3f after projective divide

      Vector3f screen = viewportTransform(clip, SCREEN_WIDTH, SCREEN_HEIGHT);

      transformedData[i] = {screen, cubeVertices[i].color, cubeVertices[i].u,
                            cubeVertices[i].v};
    }

    // draw triangles
    for (const auto &tri : cubeTriangles) {
      const auto &d0 = transformedData[tri[0]];
      const auto &d1 = transformedData[tri[1]];
      const auto &d2 = transformedData[tri[2]];

      drawTriangle(d0.screen, d1.screen, d2.screen, d0.color, d1.color,
                   d2.color, d0.u, d0.v, d1.u, d1.v, d2.u, d2.v, // UV
                   framebuffer, depthBuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // === display ===

    // update texture
    SDL_UpdateTexture(texture, nullptr, framebuffer.data(),
                      SCREEN_WIDTH * sizeof(uint32_t));

    // render texture
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);

    SDL_Delay(10);
  }

  // ********** SDL cleanup **********
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}