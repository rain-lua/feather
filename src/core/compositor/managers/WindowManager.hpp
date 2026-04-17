#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "../../../include/Defines.hpp"

class Compositor;

struct Window {
    wl_list m_Link;
	Compositor *m_Server;
	wlr_xdg_toplevel *m_XDGToplevel;
	wlr_scene_tree *m_SceneTree;
	wl_listener m_Map;
	wl_listener m_Unmap;
	wl_listener m_Commit;
	wl_listener m_Destroy;
	wl_listener m_RequestMove;
	wl_listener m_RequestResize;
	wl_listener m_RequestMaximize;
	wl_listener m_RequestFullscreen;
};

class WindowManager {
public:
    static void HandleNewWindow(wl_listener *listener, void *data);
    static void HandleWindowMap(wl_listener *listener, void *data);
    static void Tile(Compositor *server);
	static void FocusWindow(Window *window);
    static void HandleWindowUnmap(wl_listener *listener, void *data);
    static void HandleWindowCommit(wl_listener *listener, void *data);
    static void HandleWindowDestroy(wl_listener *listener, void *data);
	static void HandleWindowRequestMove(wl_listener *listener, void *data);
	static void HandleWindowRequestResize(wl_listener *listener, void *data);
	static void HandleWindowRequestMaximize(wl_listener *listener, void *data);
	static void HandleWindowRequestFullscreen(wl_listener *listener, void *data);
};

#endif