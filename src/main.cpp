#include "core/compositor/Compositor.hpp"
#include "debug/Debug.hpp"

#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
    bool allow_root = false;
    bool running_elevated = (getuid() != geteuid() || geteuid() == 0);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--this-is-fine") == 0) {
            allow_root = true;
        }
    }

    if (running_elevated && allow_root) {
        log_warn("You chose to run as root. May your backups be recent.");
    }

    if (running_elevated && !allow_root) {
        log_critical("Running with elevated privileges is forbidden unless --this-is-fine is specified.");
        return 1;
    }

    if (!getenv("XDG_RUNTIME_DIR")) {
        log_critical("XDG_RUNTIME_DIR not set, cannot create Wayland socket!");
        return 1;
    }

    g_pCompositor = std::make_unique<Compositor>();

    if (!g_pCompositor->Initialize()) {
        log_critical("Failed to initialize feather!");
        return 1;
    }

    g_pCompositor->Run();
    g_pCompositor->Cleanup();

    return 0;
}