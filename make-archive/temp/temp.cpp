#include "temp.h"
#include <unistd.h>

void RemoveTempFiles(const std::string &tempDir,
                     const std::vector<std::string> &gzPaths)
{
    for (const auto &p : gzPaths)
    {
        unlink(p.c_str());
    }
    rmdir(tempDir.c_str());
}
