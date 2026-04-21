#include "core/compositor/Compositor.hpp"
#include "debug/Debug.hpp"

int main(int argc, char **argv){
    if (!getenv("XDG_RUNTIME_DIR")) {
        log_error("XDG_RUNTIME_DIR not set, cannot create Wayland socket");
        return 1;
    }

    g_pCompositor = std::make_unique<Compositor>();

    if (!g_pCompositor->Initialize()) {
        log_critical("Failed to initialize compositor! You are on your own, have fun! :)");
        return 1;
    }

    g_pCompositor->Run();
    g_pCompositor->Cleanup();

    return 0;
}