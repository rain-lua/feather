#ifndef COMPOSITOR_H
#define COMPOSITOR_H

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
    wlr_backend* m_Backend;
    wlr_renderer* m_Renderer;
    wlr_allocator* m_Allocator;
    wlr_compositor* m_Compositor;
    wlr_subcompositor* m_SubCompositor;
    wlr_data_device_manager* m_DataDeviceManager;
    wlr_output_layout* m_OutputLayout;

    wlr_xwayland* m_XWayland;

    wlr_scene* m_Scene;
    wlr_scene_output_layout* m_SceneLayout;
    wlr_seat* m_Seat;

    wlr_xdg_shell* m_XDGShell;

    ConfigManager m_ConfigManager;
    MonitorManager m_MonitorManager;

    WindowManager m_WindowManager;
    LayoutManager m_LayoutManager;
    DecorationManager m_DecorationManager;

    InputManager m_InputManager;
    SeatManager m_SeatManager;
    KeyboardManager m_KeyboardManager;
    MouseManager m_MouseManager;
};

inline std::unique_ptr<Compositor> g_pCompositor;

#endif