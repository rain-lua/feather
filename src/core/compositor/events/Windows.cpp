#include "Events.hpp"

void Events::HandleNewWindow(wl_listener* listener, void* data) {
	Logger::Log(LogLevel::DEBUG, "New window!");

	wlr_xdg_toplevel* XDG_Toplevel = static_cast<wlr_xdg_toplevel*>(data);

	Window* window = new Window;

	window->m_XDGToplevel = XDG_Toplevel;
	window->m_SceneTree = wlr_scene_xdg_surface_create(&g_pCompositor->m_Scene->tree, XDG_Toplevel->base);
	window->m_SceneTree->node.data = window;

	XDG_Toplevel->base->data = window->m_SceneTree;

	window->m_Map.notify = Events::HandleWindowMap;
    window->m_Unmap.notify = Events::HandleWindowUnmap;
    window->m_Commit.notify = Events::HandleWindowCommit;
    window->m_Destroy.notify = Events::HandleWindowDestroy;
	window->m_RequestMove.notify = Events::HandleWindowRequestMove;
	window->m_RequestResize.notify = Events::HandleWindowRequestResize;
	window->m_RequestMaximize.notify = Events::HandleWindowRequestMaximize;
    window->m_RequestFullscreen.notify = Events::HandleWindowRequestFullscreen;

	wl_signal_add(&XDG_Toplevel->base->surface->events.map, &window->m_Map);
	wl_signal_add(&XDG_Toplevel->base->surface->events.unmap, &window->m_Unmap);
	wl_signal_add(&XDG_Toplevel->base->surface->events.commit, &window->m_Commit);

	wl_signal_add(&XDG_Toplevel->events.destroy, &window->m_Destroy);
	wl_signal_add(&XDG_Toplevel->events.request_move, &window->m_RequestMove);
	wl_signal_add(&XDG_Toplevel->events.request_resize, &window->m_RequestResize);
	wl_signal_add(&XDG_Toplevel->events.request_maximize, &window->m_RequestMaximize);
	wl_signal_add(&XDG_Toplevel->events.request_fullscreen, &window->m_RequestFullscreen);
}

void Events::HandleWindowMap(wl_listener* listener, void* data) {
	Logger::Log(LogLevel::DEBUG, "Window map");

    Window* window = wl_container_of(listener, window, m_Map);
	wl_list_insert(&g_pCompositor->m_Windows, &window->m_Link);

    g_pCompositor->m_LayoutManager->Tile();
	g_pCompositor->FocusWindow(window);
}

void Events::HandleWindowUnmap(wl_listener* listener, void* data) {
    Window* window = wl_container_of(listener, window, m_Unmap);

	wl_list_remove(&window->m_Link);
	g_pCompositor->m_LayoutManager->Tile();
}

void Events::HandleWindowCommit(wl_listener* listener, void* data) {
    Window* window = wl_container_of(listener, window, m_Commit);

	if (window->m_XDGToplevel->base->initial_commit) {
		wlr_xdg_toplevel_set_size(window->m_XDGToplevel, 0, 0);
	}
}

void Events::HandleWindowRequestMove(wl_listener* listener, void* data) {
	Window* window = wl_container_of(listener, window, m_RequestMove);
}

void Events::HandleWindowRequestResize(wl_listener* listener, void* data) {
	wlr_xdg_toplevel_resize_event* event = static_cast<wlr_xdg_toplevel_resize_event*>(data);
	Window* window = wl_container_of(listener, window, m_RequestResize);
}

void Events::HandleWindowRequestMaximize(wl_listener* listener, void* data) {
	Window* window = wl_container_of(listener, window, m_RequestMaximize);

	if (window->m_XDGToplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(window->m_XDGToplevel->base);
	}
}

void Events::HandleWindowRequestFullscreen(wl_listener* listener, void* data) {
	Window* window = wl_container_of(listener, window, m_RequestFullscreen);

	if (window->m_XDGToplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(window->m_XDGToplevel->base);
	}
}

void Events::HandleWindowDestroy(wl_listener* listener, void* data) {
    Window* window = wl_container_of(listener, window, m_Destroy);

	if (!window) {
		return;
	}

	if(!wl_list_empty(&g_pCompositor->m_Windows)) {
		g_pCompositor->FocusWindow(wl_container_of(g_pCompositor->m_Windows.prev, g_pCompositor->m_FocusedWindow, m_Link));
	} else {
		g_pCompositor->m_FocusedWindow = nullptr;
	}

	wl_list_remove(&window->m_Map.link);
	wl_list_remove(&window->m_Unmap.link);
	wl_list_remove(&window->m_Commit.link);
	wl_list_remove(&window->m_Destroy.link);
	wl_list_remove(&window->m_RequestMove.link);
	wl_list_remove(&window->m_RequestResize.link);
	wl_list_remove(&window->m_RequestMaximize.link);
	wl_list_remove(&window->m_RequestFullscreen.link);

	delete window;
}