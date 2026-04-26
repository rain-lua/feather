#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "../../../include/Defines.hpp"

struct Window {
    wl_list m_Link;

	wlr_xdg_toplevel* m_XDGToplevel;
	wlr_scene_tree* m_SceneTree;
	
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
    WindowManager();

	void Initialize();
	void Cleanup();

	wl_list m_Windows;
	wl_listener m_NewWindow;

	Window* m_FocusedWindow;

	void FocusWindow(Window* window);
	void CloseWindow(Window* window);

	Window* FindWindowAt(double lx, double ly, wlr_surface **surface, double* sx, double* sy);
	
    static void HandleNewWindow(wl_listener* listener, void* data);
    static void HandleWindowMap(wl_listener* listener, void* data);
    static void HandleWindowUnmap(wl_listener* listener, void* data);
    static void HandleWindowCommit(wl_listener* listener, void* data);
    static void HandleWindowDestroy(wl_listener* listener, void* data);
	static void HandleWindowRequestMove(wl_listener* listener, void* data);
	static void HandleWindowRequestResize(wl_listener* listener, void* data);
	static void HandleWindowRequestMaximize(wl_listener* listener, void* data);
	static void HandleWindowRequestFullscreen(wl_listener* listener, void* data);
};

#endif