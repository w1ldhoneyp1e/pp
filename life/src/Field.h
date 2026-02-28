#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Field
{
  public:
    int width;
    int height;
    std::vector<uint8_t> cells;

    Field(int width, int height);

    void set(int x, int y, bool value);

    static Field load(const std::string &filename);
    void save(const std::string &filename) const;
};
