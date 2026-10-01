#include "Windows.hpp"

#include "../../feather/Feather.hpp"
#include "../../../debug/Logger.hpp"

void HandleNewWindow(wl_listener* listener, void* data) {
    Logger::Log(LogLevel::DEBUG, "New window!");

    wlr_xdg_toplevel* XDG_Toplevel = static_cast<wlr_xdg_toplevel*>(data);

    Window* window = new Window;

    window->m_XDGToplevel = XDG_Toplevel;
    window->m_SceneTree = wlr_scene_xdg_surface_create(&g_pFeather->m_Scene->tree, XDG_Toplevel->base);

    window->m_SceneTree->node.data = window;
    XDG_Toplevel->base->data = window->m_SceneTree;

    window->m_Map.Init(
        &XDG_Toplevel->base->surface->events.map,
        window,
        HandleWindowMap
    );

    window->m_Unmap.Init(
        &XDG_Toplevel->base->surface->events.unmap,
        window,
        HandleWindowUnmap
    );

    window->m_Commit.Init(
        &XDG_Toplevel->base->surface->events.commit,
        window,
        HandleWindowCommit
    );

    window->m_Destroy.Init(
        &XDG_Toplevel->events.destroy,
        window,
        HandleWindowDestroy
    );

    window->m_RequestMove.Init(
        &XDG_Toplevel->events.request_move,
        window,
        HandleWindowRequestMove
    );

    window->m_RequestResize.Init(
        &XDG_Toplevel->events.request_resize,
        window,
        HandleWindowRequestResize
    );

    window->m_RequestMaximize.Init(
        &XDG_Toplevel->events.request_maximize,
        window,
        HandleWindowRequestMaximize
    );

    window->m_RequestFullscreen.Init(
        &XDG_Toplevel->events.request_fullscreen,
        window,
        HandleWindowRequestFullscreen
    );
}

void HandleWindowMap(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    Logger::Log(LogLevel::DEBUG, "Window map");

    wl_list_insert(
        &g_pFeather->m_Windows,
        &window->m_Link
    );

    g_pFeather->m_LayoutManager->Tile();
    g_pFeather->FocusWindow(window);
}

void HandleWindowUnmap(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    wl_list_remove(&window->m_Link);

    g_pFeather->m_LayoutManager->Tile();
}

void HandleWindowCommit(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (window->m_XDGToplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(
            window->m_XDGToplevel,
            0,
            0
        );
    }
}

void HandleWindowRequestMove(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);
}

void HandleWindowRequestResize(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    wlr_xdg_toplevel_resize_event* event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
}

void HandleWindowRequestMaximize(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (window->m_XDGToplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(
            window->m_XDGToplevel->base
        );
    }
}

void HandleWindowRequestFullscreen(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (window->m_XDGToplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(
            window->m_XDGToplevel->base
        );
    }
}

void HandleWindowDestroy(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (!window) {
        return;
    }

    if (!wl_list_empty(&g_pFeather->m_Windows)) {
        g_pFeather->FocusWindow(
            wl_container_of(
                g_pFeather->m_Windows.prev,
                g_pFeather->m_FocusedWindow,
                m_Link
            )
        );
    } else {
        g_pFeather->m_FocusedWindow = nullptr;
    }

	window->m_Map.Remove();
    window->m_Unmap.Remove();
    window->m_Commit.Remove();
    window->m_Destroy.Remove();
    window->m_RequestMove.Remove();
    window->m_RequestResize.Remove();
    window->m_RequestMaximize.Remove();
    window->m_RequestFullscreen.Remove();

    delete window;
}