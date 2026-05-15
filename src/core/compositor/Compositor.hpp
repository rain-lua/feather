#pragma once

#include "../../include/Defines.hpp"
#include "../../config/ConfigManager.hpp"
#include "../input/InputManager.hpp"
#include "../input/managers/SeatManager.hpp"
#include "../input/managers/KeyboardManager.hpp"
#include "../input/managers/MouseManager.hpp"
#include "../output/MonitorManager.hpp"
#include "./managers/WindowManager.hpp"
#include "./managers/LayoutManager.hpp"
#include "./managers/DecorationManager.hpp"

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
    std::unique_ptr<MonitorManager>    m_MonitorManager;
    std::unique_ptr<InputManager>      m_InputManager;
    std::unique_ptr<SeatManager>       m_SeatManager;
    std::unique_ptr<KeyboardManager>   m_KeyboardManager;
    std::unique_ptr<MouseManager>      m_MouseManager;
    std::unique_ptr<WindowManager>     m_WindowManager;
    std::unique_ptr<LayoutManager>     m_LayoutManager;
    std::unique_ptr<DecorationManager> m_DecorationManager;
};

inline std::unique_ptr<Compositor> g_pCompositor;