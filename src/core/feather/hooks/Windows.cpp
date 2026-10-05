#include "Windows.hpp"

#include "../../../debug/Logger.hpp"
#include "../../feather/Feather.hpp"

void HandleNewWindow(wl_listener* listener, void* data) {
    Logger::Log(LogLevel::DEBUG, "New window!");

    wlr_xdg_toplevel* xdgToplevel     = static_cast<wlr_xdg_toplevel*>(data);

    Window*           newWindow       = new Window;

    newWindow->m_xdgToplevel          = xdgToplevel;
    newWindow->m_sceneTree            = wlr_scene_xdg_surface_create(&g_Feather->m_scene->tree, xdgToplevel->base);

    newWindow->m_sceneTree->node.data = newWindow;
    xdgToplevel->base->data           = newWindow->m_sceneTree;

    newWindow->m_map.Init(&xdgToplevel->base->surface->events.map, newWindow, HandleWindowMap);
    newWindow->m_unmap.Init(&xdgToplevel->base->surface->events.unmap, newWindow, HandleWindowUnmap);
    newWindow->m_commit.Init(&xdgToplevel->base->surface->events.commit, newWindow, HandleWindowCommit);
    newWindow->m_destroy.Init(&xdgToplevel->events.destroy, newWindow, HandleWindowDestroy);
    newWindow->m_requestMove.Init(&xdgToplevel->events.request_move, newWindow, HandleWindowRequestMove);
    newWindow->m_requestResize.Init(&xdgToplevel->events.request_resize, newWindow, HandleWindowRequestResize);
    newWindow->m_requestMaximize.Init(&xdgToplevel->events.request_maximize, newWindow, HandleWindowRequestMaximize);
    newWindow->m_requestFullscreen.Init(&xdgToplevel->events.request_fullscreen, newWindow, HandleWindowRequestFullscreen);
}

void HandleWindowMap(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    Logger::Log(LogLevel::DEBUG, "Window map");

    wl_list_insert(&g_Feather->m_windows, &window->m_link);

    g_Feather->m_LayoutManager->Tile();
    g_Feather->FocusWindow(window);
}

void HandleWindowUnmap(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    wl_list_remove(&window->m_link);

    g_Feather->m_LayoutManager->Tile();
}

void HandleWindowCommit(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (window->m_xdgToplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(window->m_xdgToplevel, 0, 0);
    }
}

void HandleWindowRequestMove(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);
}

void HandleWindowRequestResize(void* owner, void* data) {
    Window*                        window = static_cast<Window*>(owner);

    wlr_xdg_toplevel_resize_event* event  = static_cast<wlr_xdg_toplevel_resize_event*>(data);
}

void HandleWindowRequestMaximize(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (window->m_xdgToplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(window->m_xdgToplevel->base);
    }
}

void HandleWindowRequestFullscreen(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (window->m_xdgToplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(window->m_xdgToplevel->base);
    }
}

void HandleWindowDestroy(void* owner, void* data) {
    Window* window = static_cast<Window*>(owner);

    if (!window) {
        return;
    }

    if (!wl_list_empty(&g_Feather->m_windows)) {
        g_Feather->FocusWindow(wl_container_of(g_Feather->m_windows.prev, g_Feather->m_focusedWindow, m_link));
    } else {
        g_Feather->m_focusedWindow = nullptr;
    }

    window->m_map.Remove();
    window->m_unmap.Remove();
    window->m_commit.Remove();
    window->m_destroy.Remove();
    window->m_requestMove.Remove();
    window->m_requestResize.Remove();
    window->m_requestMaximize.Remove();
    window->m_requestFullscreen.Remove();

    delete window;
}