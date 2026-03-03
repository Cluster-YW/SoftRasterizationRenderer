#include <SDL2/SDL.h>
#include <iostream>
#include <vector> 
#include <cstdint>
#include "core/Vector3f.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

using Framebuffer = std::vector<uint32_t>;

void put_pixel(Framebuffer& framebuffer, int x, int y, uint32_t color) {
    framebuffer[y * SCREEN_WIDTH + x] = color;
}

void draw_line(Framebuffer& framebuffer, int x0, int y0, int x1, int y1, uint32_t color) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0); // determine if the line is steep
    if (steep) { // if the line is steep, swap the x and y coordinates
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    //ensure x0 <= x1
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    // Bresenham's algorithm
    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    int err = dx / 2;
    int ystep = (y0 < y1)? 1 : -1;
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

int main(int argc, char* argv[]) {

    Vector3f a(1, 2, 3);
    Vector3f b(4, 5, 6);
    Vector3f c = a + b;
    std::cout << "a + b = " << c << std::endl;  // should output (5, 7, 9)

    float dot = a.dot(b);
    std::cout << "dot = " << dot << std::endl;  // 1*4 + 2*5 + 3*6 = 32

    Vector3f cross = a.cross(b);
    std::cout << "cross = " << cross << std::endl; // (2*6-3*5, 3*4-1*6, 1*5-2*4) = (-3, 6, -3)

    Vector3f norm = a.normalized();
    std::cout << "normalized a = " << norm << ", length = " << norm.length() << std::endl;

    return 0;

    // // ********** SDL initialization **********
    // if (SDL_Init(SDL_INIT_VIDEO) != 0){
    //     std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    //     return 1;
    // }

    // SDL_Window* window = SDL_CreateWindow("SoftRasterizationRenderer", 
    //                                         SDL_WINDOWPOS_CENTERED, 
    //                                         SDL_WINDOWPOS_CENTERED, 
    //                                         SCREEN_WIDTH, 
    //                                         SCREEN_HEIGHT, 
    //                                         SDL_WINDOW_SHOWN);
    // if (window == nullptr) {
    //     std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    //     SDL_Quit();
    //     return 1;
    // }

    // SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    // if (renderer == nullptr) {
    //     std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return 1;
    // }

    // SDL_Texture* texture = SDL_CreateTexture(renderer,
    //                                          SDL_PIXELFORMAT_ARGB8888,
    //                                          SDL_TEXTUREACCESS_STREAMING,
    //                                          SCREEN_WIDTH, SCREEN_HEIGHT);
    // if (texture == nullptr) {
    //     std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
    //     SDL_DestroyRenderer(renderer);
    //     SDL_DestroyWindow(window);
    //     SDL_Quit();
    //     return 1;
    // }

    // // ********** SoftRasterizationRenderer initialization **********

    // Framebuffer framebuffer(SCREEN_WIDTH * SCREEN_HEIGHT, 0x00000000);

    // bool quit = false;
    // SDL_Event event;
    
    // const uint32_t RED = 0xFFFF0000;
    // const uint32_t GREEN = 0xFF00FF00;
    // const uint32_t BLUE = 0xFF0000FF;

    // // ********** Main loop **********
    // while (!quit) {

    //     // event handling
    //     while (SDL_PollEvent( &event )) {
    //         if (event.type == SDL_QUIT) {
    //             quit = true;
    //         }
    //     }

    //     // rendering
    //     // clear framebuffer
    //     std::fill(framebuffer.begin(), framebuffer.end(), 0x00000000);

    //     int center_x = SCREEN_WIDTH / 2;
    //     int center_y = SCREEN_HEIGHT / 2;
    //     // draw a cross
    //     draw_line(framebuffer, center_x - 50, center_y, center_x + 50, center_y, RED);
    //     draw_line(framebuffer, center_x, center_y - 50, center_x, center_y + 50, GREEN);
        
    //     // draw a rectangle
    //     draw_line(framebuffer, 100, 100, 300, 100, BLUE);
    //     draw_line(framebuffer, 300, 100, 300, 300, BLUE);
    //     draw_line(framebuffer, 300, 300, 100, 300, BLUE);
    //     draw_line(framebuffer, 100, 300, 100, 100, BLUE);
        

    //     // update texture
    //     SDL_UpdateTexture(texture, nullptr, framebuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));

    //     // render texture
    //     SDL_RenderClear(renderer);
    //     SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    //     SDL_RenderPresent(renderer);

    //     SDL_Delay(10);
    // }


    // // ********** SDL cleanup **********
    // SDL_DestroyTexture(texture);
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();
    // return 0;
}