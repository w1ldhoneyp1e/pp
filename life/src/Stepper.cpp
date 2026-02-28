#include "Stepper.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

namespace
{

void processRows(const Field &input, Field &output, int startRow, int endRow)
{
    const int w = input.width;
    const int h = input.height;

    for (int y = startRow; y < endRow; y++)
    {
        const int yPrev = (y - 1 + h) % h;
        const int yNext = (y + 1) % h;

        for (int x = 0; x < w; x++)
        {
            const int xPrev = (x - 1 + w) % w;
            const int xNext = (x + 1) % w;

            const int neighbors =
                input.cells[yPrev * w + xPrev] + input.cells[yPrev * w + x] +
                input.cells[yPrev * w + xNext] + input.cells[y * w + xPrev] +
                input.cells[y * w + xNext] + input.cells[yNext * w + xPrev] +
                input.cells[yNext * w + x] + input.cells[yNext * w + xNext];

            const int cellIndex = y * w + x;
            const bool alive = input.cells[cellIndex] != 0;

            const bool keepAliveCond = neighbors == 2 || neighbors == 3;
            const bool makeAliveCond = neighbors == 3;
            const bool shouldLive = (alive ? keepAliveCond : makeAliveCond);
            output.cells[cellIndex] = shouldLive ? 1 : 0;
        }
    }
}

} // namespace

namespace Stepper
{

std::pair<Field, double> step(const Field &field, int numThreads)
{
    Field next(field.width, field.height);

    numThreads = std::max(1, std::min(numThreads, field.height));

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::jthread> threads;
    threads.reserve(numThreads);

    const int rowsPerThread = field.height / numThreads;
    const int remainder = field.height % numThreads;

    int startRow = 0;
    for (int i = 0; i < numThreads; i++)
    {
        const int additionalRow = i < remainder ? 1 : 0;
        const int endRow = startRow + rowsPerThread + additionalRow;
        threads.emplace_back(processRows, std::cref(field), std::ref(next),
                             startRow, endRow);
        startRow = endRow;
    }

    threads.clear();

    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start).count();

    return {std::move(next), ms};
}

} // namespace Stepper
