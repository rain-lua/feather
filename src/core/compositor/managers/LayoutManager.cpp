#include "LayoutManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

LayoutManager::LayoutManager() {
    m_MasterFact = 0.5f;
}

void LayoutManager::Tile(Compositor *server) {
    if (wl_list_empty(&server->m_Windows)) {
        return;
    }

    wlr_box box;
    wlr_output_layout_get_box(server->m_OutputLayout, NULL, &box);

    int width = box.width;
    int height = box.height;

    if (wl_list_length(&server->m_Windows) == 1) {
        Window *w = wl_container_of(server->m_Windows.next, w, m_Link);
        
        wlr_scene_node_set_position(&w->m_SceneTree->node, box.x, box.y);
        wlr_xdg_toplevel_set_size(w->m_XDGToplevel, width, height);
        return;
    }

    int master_width = (int)(width * m_MasterFact);
    int stack_count = wl_list_length(&server->m_Windows) - 1;
    int stack_width = width - master_width;
    int stack_height = height / stack_count;

    Window *w;
    int i = 0;

    wl_list_for_each(w, &server->m_Windows, m_Link) {
        if (i == 0) {
            wlr_scene_node_set_position(&w->m_SceneTree->node, box.x, box.y);
            wlr_xdg_toplevel_set_size(w->m_XDGToplevel, master_width, height);
        } else {
            wlr_scene_node_set_position(&w->m_SceneTree->node,box.x + master_width, box.y + (i - 1) * stack_height);
            wlr_xdg_toplevel_set_size(w->m_XDGToplevel, stack_width, stack_height);
        }
        i++;
    }
}