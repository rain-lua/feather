#include "LayoutManager.hpp"

#include "../../debug/Logger.hpp"
#include "../../feather/Feather.hpp"

LayoutManager::LayoutManager() {
    m_layout     = g_Feather->m_ConfigManager->GetString("layout.mode");
    m_masterFact = g_Feather->m_ConfigManager->GetFloat("layout.master.factor");
}

void LayoutManager::Tile() {
    if (wl_list_empty(&g_Feather->m_windows)) {
        return;
    }

    wlr_box box;
    wlr_output_layout_get_box(g_Feather->m_outputLayout, nullptr, &box);

    int width  = box.width;
    int height = box.height;

    if (m_layout == "master") {
        if (wl_list_length(&g_Feather->m_windows) == 1) {
            Window* w = wl_container_of(g_Feather->m_windows.next, w, m_link);

            wlr_scene_node_set_position(&w->m_sceneTree->node, box.x, box.y);
            wlr_xdg_toplevel_set_size(w->m_xdgToplevel, width, height);
            return;
        }

        int     masterWidth = (int)(width * m_masterFact);
        int     stackCount  = wl_list_length(&g_Feather->m_windows) - 1;
        int     stackWidth  = width - masterWidth;
        int     stackHeight = height / stackCount;

        Window* w;
        int     i = 0;

        wl_list_for_each(w, &g_Feather->m_windows, m_link) {
            if (i == 0) {
                wlr_scene_node_set_position(&w->m_sceneTree->node, box.x, box.y);
                wlr_xdg_toplevel_set_size(w->m_xdgToplevel, masterWidth, height);
            } else {
                wlr_scene_node_set_position(&w->m_sceneTree->node, box.x + masterWidth, box.y + (i - 1) * stackHeight);
                wlr_xdg_toplevel_set_size(w->m_xdgToplevel, stackWidth, stackHeight);
            }
            i++;
        }
    }
}