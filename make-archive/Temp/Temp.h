#pragma once

#include <optional>
#include <string>
#include <vector>

struct TempDir
{
    static std::optional<TempDir> Create();

    std::string Path() const;
    void Remove(const std::vector<std::string> &gzPaths);

private:
    explicit TempDir(std::string path);

    std::string path_;
};
