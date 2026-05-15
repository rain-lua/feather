#include "DecorationManager.hpp"
#include "../Compositor.hpp"
#include "../../../debug/Logger.hpp"
#include <cstdlib>

DecorationManager::DecorationManager() {
    Logger::Log(LogLevel::INFO, "[OK] DecorationManager");

    wl_list_init(&m_Decorations);

    m_NewDecoration.notify = HandleNewDecoration;
    wl_signal_add(&g_pCompositor->m_XDGDecorationManager->events.new_toplevel_decoration, &m_NewDecoration);
}

DecorationManager::~DecorationManager() {
    wl_list_remove(&m_NewDecoration.link);

    while (!wl_list_empty(&m_Decorations)) {
        Decoration* deco = wl_container_of(m_Decorations.next, deco, m_Link);
        DestroyDecoration(deco);
    }
}

void DecorationManager::DestroyDecoration(Decoration* deco) {
    if (!deco) {
        return;
    }

    wl_list_remove(&deco->m_RequestMode.link);
    wl_list_remove(&deco->m_Destroy.link);
    wl_list_remove(&deco->m_Link);

    delete deco;
}

void DecorationManager::HandleNewDecoration(wl_listener* listener, void* data) {
    wlr_xdg_toplevel_decoration_v1* wlr_decoration = static_cast<wlr_xdg_toplevel_decoration_v1*>(data);
    Logger::Log(LogLevel::DEBUG, "New decoration request");

    if (!wlr_decoration) { 
        return;
    }

    Decoration* deco = new Decoration;

    deco->m_Decoration = wlr_decoration;

    deco->m_RequestMode.notify = DecorationManager::HandleRequestMode;
    deco->m_Destroy.notify = DecorationManager::HandleDecorationDestroy;
    
    wl_signal_add(&wlr_decoration->events.request_mode, &deco->m_RequestMode);
    wl_signal_add(&wlr_decoration->events.destroy, &deco->m_Destroy);

    wl_list_insert(&g_pCompositor->m_DecorationManager->m_Decorations, &deco->m_Link);
}

void DecorationManager::HandleRequestMode(wl_listener* listener, void* data) {
    Decoration* deco = wl_container_of(listener, deco, m_RequestMode);

    if (!deco || !deco->m_Decoration) {
        return;
    }
    
    Logger::Log(LogLevel::DEBUG, "Client requested mode: %d", deco->m_Decoration->requested_mode);
    
    if (deco->m_Decoration->requested_mode != WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE) {
        Logger::Log(LogLevel::DEBUG, "Forcing SSD");
        wlr_xdg_toplevel_decoration_v1_set_mode(deco->m_Decoration, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }
}

void DecorationManager::HandleDecorationDestroy(wl_listener* listener, void* data) {
    Decoration* deco = wl_container_of(listener, deco, m_Destroy);

    g_pCompositor->m_DecorationManager->DestroyDecoration(deco);
}