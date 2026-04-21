#include "DecorationManager.hpp"
#include "../Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include <cstdlib>

DecorationManager::DecorationManager() {

}

void DecorationManager::Initialize() {
    m_XDGDecorationManager = wlr_xdg_decoration_manager_v1_create(g_pCompositor->m_Display);
    
    m_NewDecoration.notify = DecorationManager::HandleNewDecoration;
    wl_signal_add(&m_XDGDecorationManager->events.new_toplevel_decoration, &m_NewDecoration);
}

void DecorationManager::Cleanup() {
    wl_list_remove(&m_NewDecoration.link);
}

void DecorationManager::HandleNewDecoration(wl_listener *listener, void *data) {
    wlr_xdg_toplevel_decoration_v1 *wlr_decoration = static_cast<wlr_xdg_toplevel_decoration_v1 *>(data);
    log_debug("New decoration request %p", wlr_decoration);

    if (!wlr_decoration) return;

    Decoration *deco = (Decoration *)calloc(1, sizeof(*deco));
    if (!deco) return;

    deco->m_Decoration = wlr_decoration;

    deco->m_RequestMode.notify = DecorationManager::HandleRequestMode;
    deco->m_Destroy.notify = DecorationManager::HandleDecorationDestroy;
    
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

void DecorationManager::HandleDecorationDestroy(wl_listener *listener, void *data) {
    Decoration *deco = wl_container_of(listener, deco, m_Destroy);
    log_debug("Decoration destroyed");

    wl_list_remove(&deco->m_RequestMode.link);
    wl_list_remove(&deco->m_Destroy.link);

    free(deco);
}