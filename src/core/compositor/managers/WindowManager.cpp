#include "WindowManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

void WindowManager::HandleNewWindow(wl_listener *listener, void *data) {
	log_debug("New window!");
    Compositor *server = wl_container_of(listener, server, m_NewWindow);
	wlr_xdg_toplevel *XDG_Toplevel = static_cast<wlr_xdg_toplevel *>(data);

	Window *window = (Window *)calloc(1, sizeof(*window));
	window->m_Server = server;
	window->m_XDGToplevel = XDG_Toplevel;
	window->m_SceneTree = wlr_scene_xdg_surface_create(&window->m_Server->m_Scene->tree, XDG_Toplevel->base);
	window->m_SceneTree->node.data = window;
	XDG_Toplevel->base->data = window->m_SceneTree;

	window->m_Map.notify = WindowManager::HandleWindowMap;
    window->m_Unmap.notify = WindowManager::HandleWindowUnmap;
    window->m_Commit.notify = WindowManager::HandleWindowCommit;
    window->m_Destroy.notify = WindowManager::HandleWindowDestroy;
	window->m_RequestMove.notify = WindowManager::HandleWindowRequestMove;
	window->m_RequestResize.notify = WindowManager::HandleWindowRequestResize;
	window->m_RequestMaximize.notify = WindowManager::HandleWindowRequestMaximize;
    window->m_RequestFullscreen.notify = WindowManager::HandleWindowRequestFullscreen;

	wl_signal_add(&XDG_Toplevel->base->surface->events.map, &window->m_Map);
	wl_signal_add(&XDG_Toplevel->base->surface->events.unmap, &window->m_Unmap);
	wl_signal_add(&XDG_Toplevel->base->surface->events.commit, &window->m_Commit);
	wl_signal_add(&XDG_Toplevel->events.destroy, &window->m_Destroy);
	wl_signal_add(&XDG_Toplevel->events.request_move, &window->m_RequestMove);
	wl_signal_add(&XDG_Toplevel->events.request_resize, &window->m_RequestResize);
	wl_signal_add(&XDG_Toplevel->events.request_maximize, &window->m_RequestMaximize);
	wl_signal_add(&XDG_Toplevel->events.request_fullscreen, &window->m_RequestFullscreen);
}

void WindowManager::HandleWindowMap(wl_listener *listener, void *data) {
	log_debug("Window map");
    Window *window = wl_container_of(listener, window, m_Map);

	wl_list_insert(&window->m_Server->m_Windows, &window->m_Link);

	WindowManager::FocusWindow(window);
}

void WindowManager::FocusWindow(Window *window) {
	if (window == NULL) {
		return;
	}
	Compositor *server = window->m_Server;
	wlr_seat *seat = server->m_Seat;
	wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	wlr_surface *surface = window->m_XDGToplevel->base->surface;

	if (prev_surface == surface) {
		return;
	}

	if (prev_surface) {
		struct wlr_xdg_toplevel *prev_window = wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
		if (prev_window != NULL) {
			wlr_xdg_toplevel_set_activated(prev_window, false);
		}
	}

	wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

	wlr_scene_node_raise_to_top(&window->m_SceneTree->node);
	wl_list_remove(&window->m_Link);
	wl_list_insert(&server->m_Windows, &window->m_Link);
	
	server->m_FocusedWindow = window;
	wlr_xdg_toplevel_set_activated(window->m_XDGToplevel, true);

	if (keyboard != NULL) {
	    wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
	}
}

void WindowManager::HandleWindowUnmap(wl_listener *listener, void *data) {
    Window *window = wl_container_of(listener, window, m_Unmap);

	wl_list_remove(&window->m_Link);
}

void WindowManager::HandleWindowCommit(wl_listener *listener, void *data) {
    Window *window = wl_container_of(listener, window, m_Commit);

	if (window->m_XDGToplevel->base->initial_commit) {
		wlr_xdg_toplevel_set_size(window->m_XDGToplevel, 0, 0);
	}
}

void WindowManager::HandleWindowRequestMove(wl_listener *listener, void *data) {
	Window *window = wl_container_of(listener, window, m_RequestMove);
}

void WindowManager::HandleWindowRequestResize(wl_listener *listener, void *data) {
	wlr_xdg_toplevel_resize_event *event = static_cast<wlr_xdg_toplevel_resize_event *>(data);
	Window *window = wl_container_of(listener, window, m_RequestResize);
}

void WindowManager::HandleWindowRequestMaximize(wl_listener *listener, void *data) {
	Window *window = wl_container_of(listener, window, m_RequestMaximize);
	if (window->m_XDGToplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(window->m_XDGToplevel->base);
	}
}

void WindowManager::HandleWindowRequestFullscreen(wl_listener *listener, void *data) {
	Window *window = wl_container_of(listener, window, m_RequestFullscreen);
	if (window->m_XDGToplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(window->m_XDGToplevel->base);
	}
}

void WindowManager::HandleWindowDestroy(wl_listener *listener, void *data) {
	log_debug("Window destroyed");
    Window *window = wl_container_of(listener, window, m_Destroy);
	Compositor *server = window->m_Server;

	//we will just set it to nullptr for now to prevent issues.
	server->m_FocusedWindow = nullptr;

	wl_list_remove(&window->m_Map.link);
	wl_list_remove(&window->m_Unmap.link);
	wl_list_remove(&window->m_Commit.link);
	wl_list_remove(&window->m_Destroy.link);
	wl_list_remove(&window->m_RequestMove.link);
	wl_list_remove(&window->m_RequestResize.link);
	wl_list_remove(&window->m_RequestMaximize.link);
	wl_list_remove(&window->m_RequestFullscreen.link);

	free(window);
}