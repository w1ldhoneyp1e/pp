#include "Temp.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>

std::optional<TempDir> TempDir::Create()
{
    char buf[] = "/tmp/make-archive.XXXXXX";
    if (mkdtemp(buf) == nullptr)
    {
        std::cerr << "make-archive: mkdtemp failed: " << std::strerror(errno)
                  << std::endl;
        return std::nullopt;
    }
    return TempDir(std::string(buf));
}

TempDir::TempDir(std::string path) : path_(std::move(path)) {}

std::string TempDir::Path() const
{
    return path_;
}

void TempDir::Remove(const std::vector<std::string> &gzPaths)
{
    for (const auto &p : gzPaths)
    {
        unlink(p.c_str());
    }
    rmdir(path_.c_str());
}
