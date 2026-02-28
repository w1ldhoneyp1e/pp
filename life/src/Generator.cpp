#include "Generator.h"

#include <random>

namespace Generator
{

Field generate(int width, int height, double probability)
{
    Field field(width, height);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            field.set(x, y, dist(gen) < probability);
        }
    }

    return field;
}

} // namespace Generator
