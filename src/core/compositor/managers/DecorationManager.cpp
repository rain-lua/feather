#include "DecorationManager.hpp"
#include "../Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include <cstdlib>

void DecorationManager::HandleNewDecoration(wl_listener *listener, void *data) {
    wlr_xdg_toplevel_decoration_v1 *wlr_decoration = static_cast<wlr_xdg_toplevel_decoration_v1 *>(data);
    log_debug("New decoration request %p", wlr_decoration);

    if (!wlr_decoration) return;

    Decoration *deco = (Decoration *)calloc(1, sizeof(*deco));
    if (!deco) return;

    deco->m_Decoration = wlr_decoration;

    deco->m_RequestMode.notify = DecorationManager::HandleRequestMode;
    deco->m_Destroy.notify = DecorationManager::HandleDestroy;
    
    wl_signal_add(&wlr_decoration->events.request_mode, &deco->m_RequestMode);
    wl_signal_add(&wlr_decoration->events.destroy, &deco->m_Destroy);
}

void DecorationManager::HandleRequestMode(wl_listener *listener, void *data) {
    Decoration *deco = wl_container_of(listener, deco, m_RequestMode);
    if (!deco || !deco->m_Decoration) return;
    
    log_debug("Client requested mode: %d", deco->m_Decoration->requested_mode);
    
    if (deco->m_Decoration->requested_mode != WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
        log_debug("Forcing SSD");
        wlr_xdg_toplevel_decoration_v1_set_mode(deco->m_Decoration, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
}

void DecorationManager::HandleDestroy(wl_listener *listener, void *data) {
    Decoration *deco = wl_container_of(listener, deco, m_Destroy);
    log_debug("Decoration destroyed");

    wl_list_remove(&deco->m_RequestMode.link);
    wl_list_remove(&deco->m_Destroy.link);

    free(deco);
}