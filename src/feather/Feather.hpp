#pragma once

#include "../config/ConfigManager.hpp"
#include "../include/Defines.hpp"

#include "./classes/Listener.hpp"
#include "./handlers/InputHandler.hpp"
#include "./layout/LayoutManager.hpp"

struct Monitor {
    wl_list     m_link;
    wlr_output* m_output;

    Listener    m_frame;
    Listener    m_requestState;
    Listener    m_destroy;
};

struct Keyboard {
    wl_list       m_link;
    wlr_keyboard* m_keyboard;

    Listener      m_modifiers;
    Listener      m_key;
    Listener      m_destroy;
};

struct Pointer {
    wlr_input_device* m_device;

    Listener          m_destroy;
    wl_list           m_link;
};

enum CursorMode {
    CURSOR_PASSTHROUGH,
    CURSOR_MOVE,
    CURSOR_RESIZE,
};

struct Window {
    wl_list           m_link;

    wlr_xdg_toplevel* m_xdgToplevel;
    wlr_scene_tree*   m_sceneTree;

    Listener          m_map;
    Listener          m_unmap;
    Listener          m_commit;
    Listener          m_destroy;
    Listener          m_requestMove;
    Listener          m_requestResize;
    Listener          m_requestMaximize;
    Listener          m_requestFullscreen;
};

class Feather {
  public:
    Feather();
    ~Feather();

    bool                           Initialize();
    void                           Run();

    void                           Stop();
    void                           Cleanup();

    void                           CreateXWayland();

    wl_display*                    m_display;
    wl_event_loop*                 m_eventLoop;

    wlr_backend*                   m_backend;
    wlr_renderer*                  m_renderer;

    wl_event_source*               m_sigIntSource;
    wl_event_source*               m_sigTermSource;

    wlr_allocator*                 m_allocator;
    wlr_compositor*                m_compositor;
    wlr_subcompositor*             m_subcompositor;
    wlr_data_device_manager*       m_dataDeviceManager;
    wlr_output_layout*             m_outputLayout;

    wlr_xwayland*                  m_xwayland;

    wlr_scene*                     m_scene;
    wlr_scene_output_layout*       m_sceneLayout;

    wlr_xdg_shell*                 m_xdgShell;
    wlr_xdg_decoration_manager_v1* m_xdgDecorationManager;

    wlr_cursor*                    m_cursor;
    wlr_xcursor_manager*           m_xcursorManager;

    wlr_seat*                      m_seat;

    std::unique_ptr<InputHandler>  m_InputHandler;
    std::unique_ptr<ConfigManager> m_ConfigManager;
    std::unique_ptr<LayoutManager> m_LayoutManager;

    bool                           m_cleaningUp;

    Window*                        FindWindowAt(double lx, double ly, wlr_surface** surface, double* sx, double* sy);

    void                           FocusWindow(Window* window);
    void                           CloseWindow(Window* window);

    Window*                        m_focusedWindow;

    CursorMode                     m_cursorMode;

    wl_list                        m_outputs;
    wl_list                        m_windows;
    wl_list                        m_pointers;
    wl_list                        m_keyboards;

    WLLISTENER(m_newOutput);
    WLLISTENER(m_newWindow);
    WLLISTENER(m_newInput);

    WLLISTENER(m_cursorMotion);
    WLLISTENER(m_cursorMotionAbsolute);
    WLLISTENER(m_cursorButton);
    WLLISTENER(m_cursorAxis);
    WLLISTENER(m_cursorFrame);

    WLLISTENER(m_requestCursor);
    WLLISTENER(m_pointerFocusChange);
    WLLISTENER(m_requestSetSelection);
};

inline std::unique_ptr<Feather> g_Feather;