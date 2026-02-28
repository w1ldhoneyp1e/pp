#include "Visualizer.h"

#include "Stepper.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Visualizer {

void run(Field field, int numThreads) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
    }

    const int maxWinW = 1200;
    const int maxWinH = 900;

    int cellSize = std::max(1, std::min(maxWinW / field.width, maxWinH / field.height));
    int winW = field.width * cellSize;
    int winH = field.height * cellSize;

    SDL_Window* window = SDL_CreateWindow(
        "Game of Life",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        winW, winH,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        field.width, field.height
    );
    if (!texture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateTexture failed: ") + SDL_GetError());
    }

    std::vector<double> recentTimes;
    auto lastTitleUpdate = std::chrono::steady_clock::now();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }

        void* pixels;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);

        uint8_t* dst = static_cast<uint8_t*>(pixels);
        for (int y = 0; y < field.height; y++) {
            for (int x = 0; x < field.width; x++) {
                uint8_t color = field.cells[y * field.width + x] ? 0 : 255;
                dst[y * pitch + x * 3 + 0] = color;
                dst[y * pitch + x * 3 + 1] = color;
                dst[y * pitch + x * 3 + 2] = color;
            }
        }

        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        auto [nextField, ms] = Stepper::step(field, numThreads);
        field = std::move(nextField);
        recentTimes.push_back(ms);

        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - lastTitleUpdate).count();
        if (elapsed >= 1.0 && !recentTimes.empty()) {
            double avg = std::accumulate(recentTimes.begin(), recentTimes.end(), 0.0) / recentTimes.size();

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << "Game of Life | Avg step: " << avg << " ms | Threads: " << numThreads;
            SDL_SetWindowTitle(window, oss.str().c_str());

            recentTimes.clear();
            lastTitleUpdate = now;
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

}
