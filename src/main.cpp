#include "debug/Logger.hpp"
#include "feather/Feather.hpp"

#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
    bool allowRoot       = false;
    bool runningElevated = (getuid() != geteuid() || geteuid() == 0);

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--allow-root") == 0) {
            allowRoot = true;
        }
    }

    if (runningElevated && !allowRoot) {
        Logger::Log(LogLevel::CRITICAL, "Running with elevated privileges is forbidden unless --allow-root is specified.");
        return 1;
    }

    if (!getenv("XDG_RUNTIME_DIR")) {
        Logger::Log(LogLevel::CRITICAL, "XDG_RUNTIME_DIR not set, cannot create Wayland socket!");
        return 1;
    }

    try {
        g_Feather = std::make_unique<Feather>();
    } catch (const std::exception& e) {
        Logger::Log(LogLevel::CRITICAL, e.what());
        return 1;
    }

    if (!g_Feather->Initialize()) {
        Logger::Log(LogLevel::CRITICAL, "Failed to initialize feather!");
        return 1;
    }

    g_Feather->Run();
    g_Feather->Cleanup();

    return 0;
}