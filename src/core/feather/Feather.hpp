#pragma once

#include "../../include/Defines.hpp"
#include "../../config/ConfigManager.hpp"

#include "./classes/Listener.hpp"
#include "./handlers/InputHandler.hpp"
#include "./layout/LayoutManager.hpp"

struct Monitor {
    wl_list m_Link;
    wlr_output* m_WlrOutput;

    Listener m_Frame;
    Listener m_RequestState;
    Listener m_Destroy;
};

struct Keyboard {
    wl_list m_Link;
    wlr_keyboard* m_WlrKeyboard;

    Listener m_Modifiers;
    Listener m_Key;
    Listener m_Destroy;
};

struct Pointer {
    wlr_input_device* m_Device;

    Listener m_Destroy;
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

    Listener m_Map;
    Listener m_Unmap;
    Listener m_Commit;
    Listener m_Destroy;
    Listener m_RequestMove;
    Listener m_RequestResize;
    Listener m_RequestMaximize;
    Listener m_RequestFullscreen;
};

class Feather {
public:
    Feather();
    ~Feather();

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

    std::unique_ptr<InputHandler> m_InputHandler;
    std::unique_ptr<ConfigManager> m_ConfigManager;
    std::unique_ptr<LayoutManager> m_LayoutManager;

    Window* FindWindowAt(double lx, double ly, wlr_surface** surface, double* sx, double* sy);

    void FocusWindow(Window* window);
    void CloseWindow(Window* window);

    Window* m_FocusedWindow;

    CursorMode m_CursorMode;

    wl_list m_Outputs;
    wl_list m_Windows;
    wl_list m_Pointers;
    wl_list m_Keyboards;

    WLLISTENER(m_NewOutput);
    WLLISTENER(m_NewWindow);
    WLLISTENER(m_NewInput);

    WLLISTENER(m_CursorMotion);
    WLLISTENER(m_CursorMotionAbsolute);
    WLLISTENER(m_CursorButton);
    WLLISTENER(m_CursorAxis);
    WLLISTENER(m_CursorFrame);

    WLLISTENER(m_RequestCursor);
    WLLISTENER(m_PointerFocusChange);
    WLLISTENER(m_RequestSetSelection);
};

inline std::unique_ptr<Feather> g_pFeather;