#include "MonitorManager.hpp"
#include "../compositor/Compositor.hpp"
#include "../../debug/Debug.hpp"

MonitorManager::MonitorManager() {
    wl_list_init(&m_Outputs);
}

void MonitorManager::Initialize() {
    m_NewOutput.notify = MonitorManager::HandleNewOutput;
    wl_signal_add(&g_pCompositor->m_Backend->events.new_output, &m_NewOutput);
}

void MonitorManager::Cleanup() {
    wl_list_remove(&m_NewOutput.link);
}

void MonitorManager::HandleNewOutput(wl_listener *listener, void *data) {
    wlr_output *wlr_output = static_cast<struct wlr_output *>(data);
    log_info("--- New Monitor Connected: %s ---", wlr_output->name);

    wlr_output_init_render(wlr_output, g_pCompositor->m_Allocator, g_pCompositor->m_Renderer);

    wlr_output_state state;

    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);

    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    Monitor *monitor = new Monitor();
    monitor->m_WlrOutput = wlr_output;

    monitor->m_Frame.notify = MonitorManager::HandleOutputFrame;
    monitor->m_RequestState.notify = MonitorManager::HandleOutputRequestState;
    monitor->m_Destroy.notify = MonitorManager::HandleOutputDestroy;

    wl_signal_add(&wlr_output->events.frame, &monitor->m_Frame);
    wl_signal_add(&wlr_output->events.request_state, &monitor->m_RequestState);
    wl_signal_add(&wlr_output->events.destroy, &monitor->m_Destroy);

    wl_list_insert(&g_pCompositor->m_MonitorManager.m_Outputs, &monitor->m_Link);

    wlr_output_layout_output *l_output = wlr_output_layout_add_auto(g_pCompositor->m_OutputLayout, wlr_output);
    wlr_scene_output *scene_output = wlr_scene_output_create(g_pCompositor->m_Scene, wlr_output);

    wlr_scene_output_layout_add_output(g_pCompositor->m_SceneLayout, l_output, scene_output);
}

void MonitorManager::HandleOutputDestroy(wl_listener *listener, void *data) {
    Monitor *monitor = wl_container_of(listener, monitor, m_Destroy);
    log_info("--- Monitor Disconnected ---");

    wl_list_remove(&monitor->m_Frame.link);
    wl_list_remove(&monitor->m_RequestState.link);
    wl_list_remove(&monitor->m_Destroy.link);
    wl_list_remove(&monitor->m_Link);

    delete monitor;
}

void MonitorManager::HandleOutputRequestState(wl_listener *listener, void *data) {
    log_debug("Monitor state requested");
    Monitor *monitor = wl_container_of(listener, monitor, m_RequestState);
    
    const wlr_output_event_request_state *event = static_cast<wlr_output_event_request_state *>(data);
    wlr_output_commit_state(monitor->m_WlrOutput, event->state);
}

void MonitorManager::HandleOutputFrame(wl_listener *listener, void *data){
    Monitor *monitor = wl_container_of(listener, monitor, m_Frame);
    wlr_scene *scene = g_pCompositor->m_Scene;
    
    wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, monitor->m_WlrOutput);
    wlr_scene_output_commit(scene_output, NULL);

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}