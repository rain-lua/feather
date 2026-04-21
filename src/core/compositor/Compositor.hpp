#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "../../include/Defines.hpp"
#include "../../config/ConfigManager.hpp"
#include "../input/InputManager.hpp"
#include "../input/managers/KeyboardManager.hpp"
#include "../input/managers/MouseManager.hpp"
#include "./managers/WindowManager.hpp"
#include "./managers/LayoutManager.hpp"
#include "./managers/DecorationManager.hpp"

class Compositor {
public:
    Compositor();
    
    bool Initialize();
    void Run();
    void Cleanup();

    wl_display *m_Display;
    wlr_backend *m_Backend;
    wlr_renderer *m_Renderer;
    wlr_allocator *m_Allocator;
    wlr_compositor *m_Compositor;
    // hmmm.. SubCompositor or Subcompositor?
    wlr_subcompositor *m_SubCompositor;
    wlr_data_device_manager *m_DataDeviceManager;
    wlr_scene *m_Scene;
    wlr_scene_output_layout *m_SceneLayout;
    wlr_output_layout *m_OutputLayout;

    wlr_xdg_shell *m_XDGShell;

    InputManager m_InputManager;
    KeyboardManager m_KeyboardManager;
    WindowManager m_WindowManager;
    LayoutManager m_LayoutManager;
    DecorationManager m_DecorationManager;

	wlr_cursor *m_Cursor;
	wlr_xcursor_manager *m_CursorManager;
	wl_listener m_CursorMotion;
	wl_listener m_CursorMotionAbsolute;
	wl_listener m_CursorButton;
	wl_listener m_CursorAxis;
	wl_listener m_CursorFrame;

    wl_list m_Outputs;
    wl_list m_Pointers;

    wl_listener m_RequestCursor;
	wl_listener m_PointerFocusChange;
	wl_listener m_RequestSetSelection;
    CursorMode m_CursorMode;
    wl_listener m_NewOutput;
    wlr_seat *m_Seat;

    std::shared_ptr<FeatherConfig::ConfigManager> m_ConfigManager;
};

inline std::unique_ptr<Compositor> g_pCompositor;

#endif