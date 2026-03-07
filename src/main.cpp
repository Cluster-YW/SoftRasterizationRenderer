#define STB_IMAGE_IMPLEMENTATION

#include "SDL2/SDL_keycode.h"
#include "core/camera.h"
#include "core/matrix4x4f.h"
#include "core/mesh.h"
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

bool useTexture = true;
bool debug_ShowNormals = false;
const float NORMAL_DISPLAY_LENGTH = 0.1f;
const uint32_t NORMAL_COLOR = 0xFF00FF00; // Green

using Framebuffer = std::vector<uint32_t>;

void put_pixel(Framebuffer &framebuffer, int x, int y, uint32_t color) {
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    return;
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

  // ********* Camera initialization **********
  Camera camera(Vector3f(0.0f, 0.0f, 10.0f));

  bool firstMouse = true;
  float lastX = SCREEN_WIDTH / 2.0f;
  float lastY = SCREEN_HEIGHT / 2.0f;
  bool mouseLocked = true;

  Uint32 lastTime = SDL_GetTicks(); // time control

  // ********** SoftRasterizationRenderer initialization **********

  Framebuffer framebuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0x00000000);
  DepthBuffer depthBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);

  bool quit = false;
  SDL_Event event;
  SDL_SetRelativeMouseMode(SDL_TRUE);
  mouseLocked = true;

  const uint32_t RED = 0xFFFF0000;
  const uint32_t GREEN = 0xFF00FF00;
  const uint32_t BLUE = 0xFF0000FF;

  Mesh mesh;
  if (!mesh.loadFromObj("../mesh/suzanne.obj")) {
    std::cerr << "Failed to load mesh, falling back to procedural cube"
              << std::endl;
  }

  Texture mytexture;
  if (!mytexture.loadFromFile("../texture/mc_stone.png")) {
    std::cerr
        << "Warning: Failed to load texture, f alling back to checkerboard"
        << std::endl;
    mytexture.createCheckerboard(512, 512, 64);
  }

  Matrix4x4f modelMatrix = Matrix4x4f::identity();
  Matrix4x4f viewMatrix = Matrix4x4f::lookAt(
      Vector3f(3, 2, 5), Vector3f(0, 0, 0), Vector3f(0, 1, 0));
  Matrix4x4f projMatrix = Matrix4x4f::perspective(
      45.0f * M_PI / 180.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.0f);

  float angle = 0.0f;

  Vector3f lightDir(1.0f, -1.0f, -1.0f); // 从左上方照射
  lightDir.normalize();

  float ambient = 0.2f;

  // ********** Main loop **********
  while (!quit) {

    // ==== time control ====

    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f; // 转换为秒
    lastTime = currentTime;

    if (deltaTime > 0.1f) // avoid too large deltaTime
      deltaTime = 0.1f;

    // ==== event handling ====

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        quit = true;
      } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          mouseLocked = !mouseLocked; // toggle mouse lock
          if (mouseLocked) {
            SDL_SetRelativeMouseMode(SDL_TRUE); // hide cursor
          } else {
            SDL_SetRelativeMouseMode(SDL_FALSE); // show cursor
          }
        }
        if (event.key.keysym.sym == SDLK_n) {
          debug_ShowNormals = !debug_ShowNormals;
          std::cout << "Normal visualization: "
                    << (debug_ShowNormals ? "ON" : "OFF") << std::endl;
        }
        if (event.key.keysym.sym == SDLK_t) {
          useTexture = !useTexture;
          std::cout << "Texture: " << (useTexture ? "ON" : "OFF") << std::endl;
        }
      } else if (event.type == SDL_MOUSEMOTION && mouseLocked) {
        float xoffset = event.motion.xrel;
        float yoffset = -event.motion.yrel;

        camera.processMouseMovement(xoffset, yoffset);
      } else if (event.type == SDL_MOUSEWHEEL) {
        camera.processMouseScroll(event.wheel.y);
      }
    }

    const Uint8 *keystates = SDL_GetKeyboardState(NULL);
    if (mouseLocked) {
      if (keystates[SDL_SCANCODE_W])
        camera.processKeyboard(0, deltaTime);
      if (keystates[SDL_SCANCODE_S])
        camera.processKeyboard(1, deltaTime);
      if (keystates[SDL_SCANCODE_A])
        camera.processKeyboard(2, deltaTime);
      if (keystates[SDL_SCANCODE_D])
        camera.processKeyboard(3, deltaTime);
      if (keystates[SDL_SCANCODE_Q])
        camera.position.y -= camera.movementSpeed * deltaTime;
      if (keystates[SDL_SCANCODE_E])
        camera.position.y += camera.movementSpeed * deltaTime;
    }

    // ================
    // ==== Render ====
    std::fill(framebuffer.begin(), framebuffer.end(),
              0x00000000); // clear framebuffer
    depthBuffer.clear();   // clear depth buffer

    // update matrix
    angle += 0.05f * deltaTime;
    modelMatrix =
        Matrix4x4f::rotationY(angle) * Matrix4x4f::rotationX(angle * 0.5f);

    viewMatrix = camera.getViewMatrix();

    float aspect = static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT;
    projMatrix =
        Matrix4x4f::perspective(camera.zoom * M_PI / 180.0f, // zoom是角度
                                aspect, 0.1f, 100.0f);
    Matrix4x4f normalMat = modelMatrix.normalMatrix();

    struct NormalLine {
      Vector3f start;
      Vector3f end;
    };
    std::vector<NormalLine> normalLines;
    normalLines.reserve(mesh.vertices.size());

    // ==== Vertex Processing ====
    std::vector<VertexOut> verticesOut(mesh.vertices.size());

    for (size_t i = 0; i < mesh.vertices.size(); i++) {
      VertexOut vout;
      // model transform => to world space
      Vector3f world = modelMatrix * mesh.vertices[i].position;
      Vector3f worldNormal = normalMat * mesh.vertices[i].normal;
      worldNormal.normalize();

      // **Gouraud Shading**
      float diff = std::max(0.0f, worldNormal.dot(-lightDir));
      vout.light = Vector3f(ambient + diff, ambient + diff, ambient + diff);
      vout.albedo = mesh.vertices[i].color;

      // viewport transform
      Vector3f view = viewMatrix * world;

      // projection transform
      float wClip = -view.z; // get w
      if (std::abs(wClip) < 1e-6f)
        wClip = 1e-6f;
      vout.invW = 1.0f / wClip;

      Vector3f ndc = projMatrix * view;
      vout.screenPos = viewportTransform(ndc, SCREEN_WIDTH, SCREEN_HEIGHT);
      vout.screenPos.z = ndc.z;

      vout.texcoord = mesh.vertices[i].texcoord;
      vout.normal = worldNormal;

      // pre-calculate for later use
      vout.albedoDivW = vout.albedo * vout.invW;
      vout.lightDivW = vout.light * vout.invW;
      vout.texcoordDivW = vout.texcoord * vout.invW;

      verticesOut[i] = vout;

      if (debug_ShowNormals) {
        // vertex -> vertex + normal * length (=normal tip)
        Vector3f normalTipWorld = world + worldNormal * NORMAL_DISPLAY_LENGTH;
        Vector3f normalTipView = viewMatrix * normalTipWorld;
        Vector3f normalTipNdc = projMatrix * normalTipView;
        Vector3f normalTipScreen =
            viewportTransform(normalTipNdc, SCREEN_WIDTH, SCREEN_HEIGHT);

        normalLines.push_back({vout.screenPos, normalTipScreen});
      }
    }

    // ==== Rasterization ====
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
      const VertexOut &d0 = verticesOut[mesh.indices[i + 0]];
      const VertexOut &d1 = verticesOut[mesh.indices[i + 1]];
      const VertexOut &d2 = verticesOut[mesh.indices[i + 2]];
      drawTriangle(d0, d1, d2,               //
                   mytexture, useTexture,    //
                   framebuffer, depthBuffer, //
                   SCREEN_WIDTH, SCREEN_HEIGHT);
    }

    // ==== Debug visualization ====
    if (debug_ShowNormals) {
      for (const auto &line : normalLines) {
        int x0 = static_cast<int>(line.start.x);
        int y0 = static_cast<int>(line.start.y);
        int x1 = static_cast<int>(line.end.x);
        int y1 = static_cast<int>(line.end.y);

        // clip
        auto inScreen = [&](int x, int y) {
          return x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT;
        };

        if (inScreen(x0, y0) && inScreen(x1, y1))
          draw_line(framebuffer, x0, y0, x1, y1, NORMAL_COLOR);
      }
    }

    // === Display ===

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