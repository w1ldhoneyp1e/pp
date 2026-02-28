#include "Path.h"
#include <limits.h>
#include <unistd.h>

namespace PathHelper
{

std::string BasenameSafe(const std::string &path)
{
    size_t pos = path.rfind('/');
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string UniqueGzName(const std::string &inputPath, int index)
{
    std::string base = BasenameSafe(inputPath);
    if (base.size() > 240)
    {
        base.resize(240);
    }
    if (base.size() >= 3 && base.compare(base.size() - 3, 3, ".gz") == 0)
    {
        base.resize(base.size() - 3);
    }

    return base + "_" + std::to_string(index) + ".gz";
}

std::string GetAbsoluteArchivePath(const std::string &archivePath)
{
    if (!archivePath.empty() && archivePath[0] == '/')
    {
        return archivePath;
    }
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        return "";
    }

    return std::string(cwd) + "/" + archivePath;
}

} // namespace PathHelper
