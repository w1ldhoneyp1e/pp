#include "Archive.h"
#include "Path/Path.h"
#include <sys/wait.h>
#include <unistd.h>

bool TarRunner::Run(const std::string &archivePath,
                    const std::string &tempDir) const
{
    std::string absArchive = PathHelper::GetAbsoluteArchivePath(archivePath);
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
