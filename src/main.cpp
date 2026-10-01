#include "core/feather/Feather.hpp"
#include "debug/Logger.hpp"

#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    bool allow_root = false;
    bool running_elevated = (getuid() != geteuid() || geteuid() == 0);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--allow-root") == 0) {
            allow_root = true;
        }
    }

    if (running_elevated && !allow_root) {
        Logger::Log(LogLevel::CRITICAL, "Running with elevated privileges is forbidden unless --allow-root is specified.");
        return 1;
    }

    if (!getenv("XDG_RUNTIME_DIR")) {
        Logger::Log(LogLevel::CRITICAL, "XDG_RUNTIME_DIR not set, cannot create Wayland socket!");
        return 1;
    }

    try {
        g_pFeather = std::make_unique<Feather>();
    } catch (const std::exception& e) {
        Logger::Log(LogLevel::CRITICAL, e.what());
        return 1;
    }

    if (!g_pFeather->Initialize()) {
        Logger::Log(LogLevel::CRITICAL, "Failed to initialize feather!");
        return 1;
    }

    g_pFeather->Run();
    g_pFeather->Cleanup();

    return 0;
}