#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

void Spawn(const char* cmd) {
    pid_t pid = fork();

    if (pid < 0) {
        return;
    }

    if (pid == 0) {
        setsid();

        int fd = open("/dev/null", O_RDWR);

        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);

            if (fd > 2) {
                close(fd);
            }
        }

        execl("/bin/sh", "sh", "-c", cmd, nullptr);
        _exit(1);
    }
}