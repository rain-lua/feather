#include "Monitor.hpp"
#include "../compositor/Compositor.hpp"
#include "../../debug/Debug.hpp"

void MonitorManager::HandleNewOutput(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_NewOutput);
    wlr_output *wlr_output = static_cast<struct wlr_output *>(data);
    log_info("--- New Monitor Connected ---");

    wlr_output_init_render(wlr_output, server->m_Allocator, server->m_Renderer);

    wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    Monitor *monitor = (Monitor *)calloc(1, sizeof(*monitor));
    monitor->m_WlrOutput = wlr_output;
    monitor->m_Server = server;

    monitor->m_Frame.notify = MonitorManager::HandleOutputFrame;
    wl_signal_add(&wlr_output->events.frame, &monitor->m_Frame);

    monitor->m_RequestState.notify = MonitorManager::HandleOutputRequestState;
    wl_signal_add(&wlr_output->events.request_state, &monitor->m_RequestState);

    monitor->m_Destroy.notify = MonitorManager::HandleOutputDestroy;
    wl_signal_add(&wlr_output->events.destroy, &monitor->m_Destroy);

    wl_list_insert(&server->m_Outputs, &monitor->m_Link);

    wlr_output_layout_output *l_output = wlr_output_layout_add_auto(server->m_OutputLayout, wlr_output);
    wlr_scene_output *scene_output = wlr_scene_output_create(server->m_Scene, wlr_output);
    wlr_scene_output_layout_add_output(server->m_SceneLayout, l_output, scene_output);
}

void MonitorManager::HandleOutputDestroy(wl_listener *listener, void *data) {
    Monitor *monitor = wl_container_of(listener, monitor, m_Destroy);
    log_info("--- Monitor Disconnected ---");
    wl_list_remove(&monitor->m_Frame.link);
    wl_list_remove(&monitor->m_RequestState.link);
    wl_list_remove(&monitor->m_Destroy.link);
    wl_list_remove(&monitor->m_Link);
    free(monitor);
}

void MonitorManager::HandleOutputRequestState(wl_listener *listener, void *data) {
    log_debug("Monitor state requested");
    Monitor *monitor = wl_container_of(listener, monitor, m_RequestState);
    const wlr_output_event_request_state *event = static_cast<wlr_output_event_request_state *>(data);
    wlr_output_commit_state(monitor->m_WlrOutput, event->state);
}

void MonitorManager::HandleOutputFrame(wl_listener *listener, void *data){
    log_debug("Monitor frame callback");
    Monitor *monitor = wl_container_of(listener, monitor, m_Frame);
    wlr_scene *scene = monitor->m_Server->m_Scene;
    wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, monitor->m_WlrOutput);
    wlr_scene_output_commit(scene_output, NULL);
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}