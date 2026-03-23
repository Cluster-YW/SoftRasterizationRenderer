#include <ostream>
#define STB_IMAGE_IMPLEMENTATION

#include "render/shader.h"

#include "SDL2/SDL_keycode.h"
#include "geometry/camera.h"
#include "math/matrix4x4f.h"
#include "math/vector3f.h"
#include "render/rasterizer.h"
#include "resource/mesh.h"

#include <SDL2/SDL.h>
#include <cstdint>
#include <iostream>
#include <vector>

namespace sr {
using namespace math;     // 在 sr 内聚合 math
using namespace geometry; // 在 sr 内聚合 geometry
using namespace render;   // 在 sr 内聚合 render
} // namespace sr

using namespace sr;

bool useBlinnPhong = true;
bool wireframeMode = false;
bool useTexture = true;
bool debug_ShowNormals = false;
bool enableCulling = true;
const float NORMAL_DISPLAY_LENGTH = 0.1f;
const uint32_t NORMAL_COLOR = 0xFF00FF00; // Green

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char *argv[]) {
  Framebuffer framebuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
  DepthBuffer &depthBuffer = framebuffer.depth();

  // ********** SDL initialization **********
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window *window =
      SDL_CreateWindow("SoftRasterizationRenderer", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, framebuffer.width(),
                       framebuffer.height(), SDL_WINDOW_SHOWN);
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

  auto lambertShader = Shaders::CreateLambertShader();
  auto blinnPhongShader = Shaders::CreateBlinnPhongShader();

  float angle = 0.0f;

  Vector3f lightDir(1.0f, -1.0f, -1.0f); // 从左上方照射
  lightDir.normalize();

  float ambient = 0.2f;

  int culledCount = 0;
  int renderedCount = 0;

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
        if (event.key.keysym.sym == SDLK_c) {
          enableCulling = !enableCulling;
          std::cout << "Culling: " << (enableCulling ? "ON" : "OFF")
                    << std::endl;
        }
        if (event.key.keysym.sym == SDLK_r) {
          std::cout << "Culled triangles: " << culledCount << std::endl;
          std::cout << "Rendered triangles: " << renderedCount << std::endl;
        }
        if (event.key.keysym.sym == SDLK_l) {
          wireframeMode = !wireframeMode;
          std::cout << "Wireframe mode: " << (wireframeMode ? "ON" : "OFF")
                    << std::endl;
        }
        if (event.key.keysym.sym == SDLK_p) {
          useBlinnPhong = !useBlinnPhong;
          std::cout << "Shader: " << (useBlinnPhong ? "Blinn-Phong" : "Lambert")
                    << std::endl;
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
    framebuffer.clear(); // clear framebuffer
    depthBuffer.clear(); // clear depth buffer

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
      Varying start;
      Varying end;
    };
    std::vector<NormalLine> normalLines;
    normalLines.reserve(mesh.vertices.size());

    Uniforms uniforms;
    uniforms.model = modelMatrix;
    uniforms.view = viewMatrix;
    uniforms.proj = projMatrix;
    uniforms.normalMat = normalMat;
    uniforms.lightDir = lightDir;
    uniforms.ambient = 0.2f;
    uniforms.texture = &mytexture;
    uniforms.screenWidth = SCREEN_WIDTH;
    uniforms.screenHeight = SCREEN_HEIGHT;
    uniforms.useTexture = useTexture;
    uniforms.lightColor = Vector3f(1.0f, 1.0f, 1.0f);
    uniforms.shininess = 32.0f;
    uniforms.specularColor = Vector3f(1.0f, 1.0f, 1.0f);

    const auto &activeShader = useBlinnPhong ? blinnPhongShader : lambertShader;

    // ==== Vertex Processing ====
    std::vector<Varying> varyings(mesh.vertices.size());

    for (size_t i = 0; i < mesh.vertices.size(); i++) {
      varyings[i] = activeShader.vertexShader(mesh.vertices[i], uniforms);
    }

    // ==== Rasterization ====
    culledCount = 0;
    renderedCount = 0;

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
      int idx0 = mesh.indices[i + 0];
      int idx1 = mesh.indices[i + 1];
      int idx2 = mesh.indices[i + 2];

      const Varying &v0 = varyings[idx0];
      const Varying &v1 = varyings[idx1];
      const Varying &v2 = varyings[idx2];

      if (wireframeMode) {
        drawTriangleWireframe(v0, v1, v2, framebuffer);
        renderedCount++;
      } else {
        if (!enableCulling || isFrontFace(v0.viewPos, v1.viewPos, v2.viewPos)) {
          drawTriangle(v0, v1, v2, //
                       activeShader, uniforms, framebuffer);
          renderedCount++;
        } else
          culledCount++;
      }
    }

    // === Display ===

    // update texture
    SDL_UpdateTexture(texture, nullptr, framebuffer.data(),
                      framebuffer.width() * sizeof(uint32_t));

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