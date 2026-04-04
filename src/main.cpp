#include "core/compositor/Compositor.hpp"
#include "debug/Debug.hpp"

int main(int argc, char **argv){
    if (!getenv("XDG_RUNTIME_DIR")) {
        log_error("XDG_RUNTIME_DIR not set, cannot create Wayland socket");
        return 1;
    }

    Compositor compositor;
    if (!compositor.Initialize()) {
        log_error("failed to initialize compositor");
        return 1;
    }

    compositor.Run();
    compositor.Cleanup();
    return 0;
}