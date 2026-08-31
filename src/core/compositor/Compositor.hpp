#pragma once

#include "../../include/Defines.hpp"

#include "../../config/ConfigManager.hpp"

#include "./managers/InputManager.hpp"
#include "./managers/LayoutManager.hpp"

#include "./events/Events.hpp"
#include "../../debug/Logger.hpp"

struct Monitor {
    wl_list m_Link;
    wlr_output* m_WlrOutput;

    wl_listener m_Frame;
    wl_listener m_RequestState;
    wl_listener m_Destroy;
};

struct Keyboard {
    wl_list m_Link;
    wlr_keyboard* m_WlrKeyboard;
    
    wl_listener m_Modifiers;
    wl_listener m_Key;
    wl_listener m_Destroy;
};

struct Pointer {
    wlr_input_device* m_Device;

    wl_listener m_Destroy;
    wl_list m_Link;
};

enum CursorMode {
	CURSOR_PASSTHROUGH,
	CURSOR_MOVE,
	CURSOR_RESIZE,
};

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

class Compositor {
public:
    Compositor();
    ~Compositor();
    
    bool Initialize();
    void Run();

    void Stop();
    void Cleanup();

    bool m_CleaningUp;

    wl_display* m_Display;
    wl_event_loop* m_EventLoop;

    wlr_backend* m_Backend;
    wlr_renderer* m_Renderer;

    wl_event_source* m_SigIntSource;
    wl_event_source* m_SigTermSource;

    wlr_allocator* m_Allocator;
    wlr_compositor* m_Compositor;
    wlr_subcompositor* m_SubCompositor;
    wlr_data_device_manager* m_DataDeviceManager;
    wlr_output_layout* m_OutputLayout;

    wlr_xwayland* m_XWayland;

    wlr_scene* m_Scene;
    wlr_scene_output_layout* m_SceneLayout;

    wlr_xdg_shell* m_XDGShell;
    wlr_xdg_decoration_manager_v1* m_XDGDecorationManager;

    wlr_cursor* m_Cursor;
    wlr_xcursor_manager* m_XCursorManager;

    wlr_seat* m_Seat;

    std::unique_ptr<ConfigManager>     m_ConfigManager;
    std::unique_ptr<InputManager>      m_InputManager;
    std::unique_ptr<LayoutManager>     m_LayoutManager;

    Window* FindWindowAt(double lx, double ly, wlr_surface** surface, double* sx, double* sy);

    void FocusWindow(Window* window);
	void CloseWindow(Window* window);

    Window* m_FocusedWindow;

    CursorMode m_CursorMode;

    wl_list m_Outputs;
    wl_list m_Windows;
    wl_list m_Pointers;
    wl_list m_Keyboards;

    wl_listener m_NewOutput;
	wl_listener m_NewWindow;

    wl_listener m_NewInput;

	wl_listener m_CursorMotion;
	wl_listener m_CursorMotionAbsolute;
	wl_listener m_CursorButton;
	wl_listener m_CursorAxis;
	wl_listener m_CursorFrame;

    wl_listener m_RequestCursor;
	wl_listener m_PointerFocusChange;
	wl_listener m_RequestSetSelection;
};

inline std::unique_ptr<Compositor> g_pCompositor;