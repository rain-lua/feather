#pragma once

#include "../../../include/Defines.hpp"

struct Decoration {
    wlr_xdg_toplevel_decoration_v1* m_Decoration;

    wl_listener m_RequestMode;
    wl_listener m_Destroy;

    wl_list m_Link;
};

class DecorationManager {
public:
    DecorationManager();
    ~DecorationManager();

    wlr_xdg_decoration_manager_v1* m_XDGDecorationManager;

    wl_list m_Decorations;
    wl_listener m_NewDecoration;

    void DestroyDecoration(Decoration* deco);

    static void HandleNewDecoration(wl_listener* listener, void* data);
    static void HandleDecorationDestroy(wl_listener* listener, void* data);
    static void HandleRequestMode(wl_listener* listener, void* data);
};