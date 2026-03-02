#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // 初始化 SDL 视频子系统
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 创建窗口
    SDL_Window* window = SDL_CreateWindow("软光栅渲染器 - 环境测试",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 创建渲染器（用于在窗口上绘制 2D 内容）
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 主循环标志
    bool quit = false;
    SDL_Event event;

    // 设置初始颜色为深蓝色
    Uint8 r = 25, g = 25, b = 112, a = 255;

    // 主循环
    while (!quit) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // 清屏为深蓝色
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderClear(renderer);

        // 在此可以添加绘图命令，例如画一个红色矩形
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect rect = { 350, 250, 100, 100 };
        SDL_RenderFillRect(renderer, &rect);

        // 显示绘制结果
        SDL_RenderPresent(renderer);

        // 简单延时，降低 CPU 占用
        SDL_Delay(10);
    }

    // 清理资源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}