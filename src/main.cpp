#include <SDL2/SDL.h>
#include <iostream>
#include <vector> 
#include <cstdint>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

using Framebuffer = std::vector<uint32_t>;

void put_pixel(Framebuffer& framebuffer, int x, int y, uint32_t color) {
    framebuffer[y * SCREEN_WIDTH + x] = color;
}

int main(int argc, char* argv[]) {

    // ********** SDL initialization **********
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SoftRasterizationRenderer", 
                                            SDL_WINDOWPOS_CENTERED, 
                                            SDL_WINDOWPOS_CENTERED, 
                                            SCREEN_WIDTH, 
                                            SCREEN_HEIGHT, 
                                            SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
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

    // ********** Main loop **********
    while (!quit) {

        // event handling
        while (SDL_PollEvent( &event )) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // rendering
        // clear framebuffer
        std::fill(framebuffer.begin(), framebuffer.end(), 0x00000000);

        int center_x = SCREEN_WIDTH / 2;
        int center_y = SCREEN_HEIGHT / 2;
        for (int x=center_x-100; x<=center_x+100; x++)
            put_pixel(framebuffer, x, center_y, RED);

        for (int y=center_y-100; y<=center_y+100; y++)
            put_pixel(framebuffer, center_x, y, GREEN);

        put_pixel(framebuffer, 10, 10, BLUE);
        put_pixel(framebuffer, SCREEN_WIDTH-11, 10, BLUE);
        put_pixel(framebuffer, 10, SCREEN_HEIGHT-11, BLUE);
        put_pixel(framebuffer, SCREEN_WIDTH-11, SCREEN_HEIGHT-11, BLUE);

        // update texture
        SDL_UpdateTexture(texture, nullptr, framebuffer.data(), SCREEN_WIDTH * sizeof(uint32_t));

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