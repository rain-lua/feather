#include "LayoutManager.hpp"

#include "../../feather/Feather.hpp"
#include "../../../debug/Logger.hpp"

LayoutManager::LayoutManager() {
    m_Layout = g_pFeather->m_ConfigManager->GetString("layout.mode");
    m_MasterFact = g_pFeather->m_ConfigManager->GetFloat("layout.master.factor");
}

void LayoutManager::Tile() {
    if (wl_list_empty(&g_pFeather->m_Windows)) {
        return;
    }

    wlr_box box;
    wlr_output_layout_get_box(g_pFeather->m_OutputLayout, nullptr, &box);

    int width = box.width;
    int height = box.height;

    if (m_Layout == "master") {
        if (wl_list_length(&g_pFeather->m_Windows) == 1) {
                Window* w = wl_container_of(g_pFeather->m_Windows.next, w, m_Link);
                
                wlr_scene_node_set_position(&w->m_SceneTree->node, box.x, box.y);
                wlr_xdg_toplevel_set_size(w->m_XDGToplevel, width, height);
                return;
            }

            int master_width = (int)(width* m_MasterFact);
            int stack_count = wl_list_length(&g_pFeather->m_Windows) - 1;
            int stack_width = width - master_width;
            int stack_height = height / stack_count;

            Window* w;
            int i = 0;

            wl_list_for_each(w, &g_pFeather->m_Windows, m_Link) {
                if (i == 0) {
                    wlr_scene_node_set_position(&w->m_SceneTree->node, box.x, box.y);
                    wlr_xdg_toplevel_set_size(w->m_XDGToplevel, master_width, height);
                } else {
                    wlr_scene_node_set_position(&w->m_SceneTree->node, box.x + master_width, box.y + (i - 1) * stack_height);
                    wlr_xdg_toplevel_set_size(w->m_XDGToplevel, stack_width, stack_height);
                }
                i++;
            }
    }
}