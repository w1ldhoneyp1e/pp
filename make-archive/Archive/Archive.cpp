#include "Archive.h"
#include "../Path/Path.h"
#include <sys/wait.h>
#include <unistd.h>

bool RunTar(const std::string &archivePath, const std::string &tempDir)
{
    std::string absArchive = GetAbsoluteArchivePath(archivePath);
    if (absArchive.empty())
    {
        return false;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        return false;
    }
    if (pid == 0)
    {
        execlp("tar", "tar", "-cf", absArchive.c_str(), "-C", tempDir.c_str(),
               ".", static_cast<char *>(nullptr));
        _exit(127);
    }
    int status;
    if (waitpid(pid, &status, 0) < 0)
    {
        return false;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
