#include "Field.h"

#include <fstream>
#include <stdexcept>

Field::Field(int width, int height)
    : width(width), height(height), cells(width * height, 0)
{
}

void Field::set(int x, int y, bool value)
{
    cells[y * width + x] = value ? 1 : 0;
}

Field Field::load(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    int width, height;
    file >> width >> height;
    file.ignore();

    Field field(width, height);

    std::string line;
    for (int y = 0; y < height; y++)
    {
        if (std::getline(file, line))
        {
            for (int x = 0; x < width; x++)
            {
                if (x < static_cast<int>(line.size()))
                {
                    field.set(x, y, line[x] == '#');
                }
            }
        }
    }

    return field;
}

void Field::save(const std::string &filename) const
{
    std::ofstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Cannot write to file: " + filename);
    }

    file << width << " " << height << "\n";

    for (int y = 0; y < height; y++)
    {
        std::string line(width, ' ');
        for (int x = 0; x < width; x++)
        {
            if (cells[y * width + x])
            {
                line[x] = '#';
            }
        }

        size_t last = line.find_last_not_of(' ');
        if (last != std::string::npos)
        {
            line = line.substr(0, last + 1);
        }
        else
        {
            line = "";
        }

        file << line << "\n";
    }
}
