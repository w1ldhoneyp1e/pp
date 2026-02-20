#include "compression.h"
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

bool CompressOne(const char *inputPath, const char *gzPath)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        return false;
    }
    if (pid == 0)
    {
        int fd = open(gzPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            _exit(127);
        }
        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            close(fd);
            _exit(127);
        }
        close(fd);
        execlp("gzip", "gzip", "-c", inputPath, static_cast<char *>(nullptr));
        _exit(127);
    }
    int status;
    if (waitpid(pid, &status, 0) < 0)
    {
        return false;
    }

    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}
