#include "WindowManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

WindowManager::WindowManager() {
	wl_list_init(&m_Windows);

	m_FocusedWindow = nullptr;
}

void WindowManager::Initialize() {
	m_NewWindow.notify = WindowManager::HandleNewWindow;
	wl_signal_add(&g_pCompositor->m_XDGShell->events.new_toplevel, &m_NewWindow);
}

void WindowManager::Cleanup() {
	wl_list_remove(&m_NewWindow.link);
}

void WindowManager::FocusWindow(Window* window) {
	if (window == nullptr) {
		return;
	}
	
	wlr_seat* seat = g_pCompositor->m_SeatManager.m_Seat;
	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;
	wlr_surface* surface = window->m_XDGToplevel->base->surface;

	if (prev_surface == surface) {
		return;
	}

	if (prev_surface) {
		wlr_xdg_toplevel* prev_window = wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
		
		if (prev_window != nullptr) {
			wlr_xdg_toplevel_set_activated(prev_window, false);
		}
	}

	wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

	wlr_scene_node_raise_to_top(&window->m_SceneTree->node);
	wl_list_remove(&window->m_Link);
	wl_list_insert(&g_pCompositor->m_WindowManager.m_Windows, &window->m_Link);
	
	g_pCompositor->m_WindowManager.m_FocusedWindow = window;
	wlr_xdg_toplevel_set_activated(window->m_XDGToplevel, true);

	if (keyboard != nullptr) {
        wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
	}
}

void WindowManager::CloseWindow(Window* window) {
	if (window == nullptr || window->m_XDGToplevel == nullptr) {
        return;
    }
    
    wlr_xdg_toplevel_send_close(window->m_XDGToplevel);
}

Window* WindowManager::FindWindowAt(double lx, double ly, wlr_surface** surface, double* sx, double* sy) {
	wlr_scene_node* node = wlr_scene_node_at( &g_pCompositor->m_Scene->tree.node, lx, ly, sx, sy);

	if (node == nullptr || node->type != WLR_SCENE_NODE_BUFFER) {
		return nullptr;
	}

	wlr_scene_buffer* scene_buffer = wlr_scene_buffer_from_node(node);
	wlr_scene_surface* scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);

	if (!scene_surface) {
		return nullptr;
	}

	*surface = scene_surface->surface;

	wlr_scene_tree* tree = node->parent;

	while (tree != nullptr && tree->node.data == nullptr) {
		tree = tree->node.parent;
	}

	return static_cast<Window*>(tree->node.data);
}

void WindowManager::HandleNewWindow(wl_listener* listener, void* data) {
	log_debug("New window!");

	wlr_xdg_toplevel* XDG_Toplevel = static_cast<wlr_xdg_toplevel *>(data);

	Window* window = new Window;
	window->m_XDGToplevel = XDG_Toplevel;
	window->m_SceneTree = wlr_scene_xdg_surface_create(&g_pCompositor->m_Scene->tree, XDG_Toplevel->base);
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

void WindowManager::HandleWindowMap(wl_listener* listener, void* data) {
	log_debug("Window map");

    Window* window = wl_container_of(listener, window, m_Map);
	wl_list_insert(&g_pCompositor->m_WindowManager.m_Windows, &window->m_Link);

    g_pCompositor->m_LayoutManager.Tile();
	g_pCompositor->m_WindowManager.FocusWindow(window);
}

void WindowManager::HandleWindowUnmap(wl_listener* listener, void* data) {
    Window* window = wl_container_of(listener, window, m_Unmap);

	wl_list_remove(&window->m_Link);
	g_pCompositor->m_LayoutManager.Tile();
}

void WindowManager::HandleWindowCommit(wl_listener* listener, void* data) {
    Window* window = wl_container_of(listener, window, m_Commit);

	if (window->m_XDGToplevel->base->initial_commit) {
		wlr_xdg_toplevel_set_size(window->m_XDGToplevel, 0, 0);
	}
}

void WindowManager::HandleWindowRequestMove(wl_listener* listener, void* data) {
	Window* window = wl_container_of(listener, window, m_RequestMove);
}

void WindowManager::HandleWindowRequestResize(wl_listener* listener, void* data) {
	wlr_xdg_toplevel_resize_event* event = static_cast<wlr_xdg_toplevel_resize_event *>(data);
	Window* window = wl_container_of(listener, window, m_RequestResize);
}

void WindowManager::HandleWindowRequestMaximize(wl_listener* listener, void* data) {
	Window* window = wl_container_of(listener, window, m_RequestMaximize);

	if (window->m_XDGToplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(window->m_XDGToplevel->base);
	}
}

void WindowManager::HandleWindowRequestFullscreen(wl_listener* listener, void* data) {
	Window* window = wl_container_of(listener, window, m_RequestFullscreen);

	if (window->m_XDGToplevel->base->initialized) {
		wlr_xdg_surface_schedule_configure(window->m_XDGToplevel->base);
	}
}

void WindowManager::HandleWindowDestroy(wl_listener* listener, void* data) {
	log_debug("Window destroyed");

    Window* window = wl_container_of(listener, window, m_Destroy);

	if(!wl_list_empty(&g_pCompositor->m_WindowManager.m_Windows)) {
		g_pCompositor->m_WindowManager.FocusWindow(wl_container_of(g_pCompositor->m_WindowManager.m_Windows.prev, g_pCompositor->m_WindowManager.m_FocusedWindow, m_Link));
	} else {
		// we will just set it to nullptr for now to prevent issues.
		g_pCompositor->m_WindowManager.m_FocusedWindow = nullptr;
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