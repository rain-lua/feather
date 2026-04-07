#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "../../include/Defines.hpp"
#include "../../config/ConfigManager.hpp"
#include "../input/manager/MouseManager.hpp"
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
    wlr_scene *m_Scene;
    wlr_scene_output_layout *m_SceneLayout;
    wlr_output_layout *m_OutputLayout;

	struct wlr_cursor *m_Cursor;
	struct wlr_xcursor_manager *m_CursorManager;
	struct wl_listener m_CursorMotion;
	struct wl_listener m_CursorMotionAbsolute;
	struct wl_listener m_CursorButton;
	struct wl_listener m_CursorAxis;
	struct wl_listener m_CursorFrame;

    wl_list m_Outputs;
    wl_list m_Keyboards;
    wl_list m_Pointers;

    struct wl_listener m_RequestCursor;
	struct wl_listener m_PointerFocusChange;
	struct wl_listener m_RequestSetSelection;
    enum CursorMode m_CursorMode;
    wl_listener m_NewOutput;
    wl_listener m_NewInput;
    wlr_seat *m_Seat;

    std::shared_ptr<FeatherConfig::ConfigManager> m_ConfigManager;
};

#endif