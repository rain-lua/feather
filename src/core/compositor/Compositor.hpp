#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include "../../include/Defines.hpp"

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

    wl_list m_Outputs;
    wl_list m_Keyboards;
    wl_listener m_NewOutput;
    wl_listener m_NewInput;
    wlr_seat *m_Seat;
};

#endif
