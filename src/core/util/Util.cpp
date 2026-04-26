#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <iostream>

#include "../compositor/managers/WindowManager.hpp"
#include "../../include/Defines.hpp"

void Spawn(const char* cmd) {
    pid_t pid = fork();
    if (pid < 0) return;  

    if (pid == 0) {
        pid_t sid = fork();
        if (sid < 0) _exit(1);
        if (sid > 0) _exit(0);  

        if (setsid() < 0) _exit(1); 

        execl("/bin/sh", "sh", "-c", cmd, nullptr);
        _exit(1); 
    }

    waitpid(pid, nullptr, 0);
}