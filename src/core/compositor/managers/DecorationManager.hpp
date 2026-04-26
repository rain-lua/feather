#ifndef DECORATION_MANAGER_H
#define DECORATION_MANAGER_H

#include "../../../include/Defines.hpp"

struct Decoration {
    wlr_xdg_toplevel_decoration_v1* m_Decoration;

    wl_listener m_RequestMode;
    wl_listener m_Destroy;
};

class DecorationManager {
public:
    DecorationManager();

    void Initialize();
    void Cleanup();

    wlr_xdg_decoration_manager_v1* m_XDGDecorationManager;
    wl_listener m_NewDecoration;

    static void HandleNewDecoration(wl_listener* listener, void* data);
    static void HandleRequestMode(wl_listener* listener, void* data);
    static void HandleDecorationDestroy(wl_listener* listener, void* data);
};

#endif