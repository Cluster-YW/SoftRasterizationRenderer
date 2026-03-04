#include "core/matrix4x4f.h"
#include "core/vector3f.h"
#include "viewport.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

using Framebuffer = std::vector<uint32_t>;

void put_pixel(Framebuffer &framebuffer, int x, int y, uint32_t color) {
  framebuffer[y * SCREEN_WIDTH + x] = color;
}

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

  bool quit = false;
  SDL_Event event;

  const uint32_t RED = 0xFFFF0000;
  const uint32_t GREEN = 0xFF00FF00;
  const uint32_t BLUE = 0xFF0000FF;

  std::vector<Vector3f> cubeVertices = {
      Vector3f(-1, -1, -1), // 0
      Vector3f(1, -1, -1),  // 1
      Vector3f(1, 1, -1),   // 2
      Vector3f(-1, 1, -1),  // 3
      Vector3f(-1, -1, 1),  // 4
      Vector3f(1, -1, 1),   // 5
      Vector3f(1, 1, 1),    // 6
      Vector3f(-1, 1, 1)    // 7
  };

  std::vector<std::pair<int, int>> cubeEdges = {
      {0, 1}, {1, 2}, {2, 3}, {3, 0}, // back
      {4, 5}, {5, 6}, {6, 7}, {7, 4}, // front
      {0, 4}, {1, 5}, {2, 6}, {3, 7}  // connection
  };

  Matrix4x4f modelMatrix = Matrix4x4f::identity();
  Matrix4x4f viewMatrix = Matrix4x4f::lookAt(
      Vector3f(3, 2, 5), Vector3f(0, 0, 0), Vector3f(0, 1, 0));
  Matrix4x4f projMatrix = Matrix4x4f::perspective(
      45.0f * M_PI / 180.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);

  // ********** Main loop **********
  while (!quit) {

    // event handling
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      }
    }

    // === rendering ===
    // clear framebuffer
    std::fill(framebuffer.begin(), framebuffer.end(), 0x00000000);

    // ======

    std::vector<Vector3f> transformedVertices(cubeVertices.size());
    for (size_t i = 0; i < cubeVertices.size(); i++) {
      Vector3f world = modelMatrix * cubeVertices[i];
      Vector3f view = viewMatrix * world;
      Vector3f clip = projMatrix * view;
      Vector3f ndc = clip;

      transformedVertices[i] =
          viewportTransform(ndc, SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    for (const auto &edge : cubeEdges) {
      const Vector3f &p1 = transformedVertices[edge.first];
      const Vector3f &p2 = transformedVertices[edge.second];
      draw_line(framebuffer, (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y,
                0xFFFFFFFF); // 白色线
    }

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