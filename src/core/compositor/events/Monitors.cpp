#include "Events.hpp"

void Events::HandleNewOutput(wl_listener* listener, void* data) {
    wlr_output* output = static_cast<wlr_output*>(data);
    Logger::Log(LogLevel::INFO, "--- New Monitor Connected: %s ---", output->name);

    wlr_output_init_render(output, g_pCompositor->m_Allocator, g_pCompositor->m_Renderer);

    wlr_output_state state;

    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    wlr_output_mode* mode = wlr_output_preferred_mode(output);

    if (mode != nullptr) {
        wlr_output_state_set_mode(&state, mode);
    }

    wlr_output_commit_state(output, &state);
    wlr_output_state_finish(&state);

    Monitor* monitor = new Monitor;

    monitor->m_WlrOutput = output;

    monitor->m_Frame.notify = Events::HandleOutputFrame;
    monitor->m_RequestState.notify = Events::HandleOutputRequestState;
    monitor->m_Destroy.notify = Events::HandleOutputDestroy;

    wl_signal_add(&output->events.frame, &monitor->m_Frame);
    wl_signal_add(&output->events.request_state, &monitor->m_RequestState);
    wl_signal_add(&output->events.destroy, &monitor->m_Destroy);

    wl_list_insert(&g_pCompositor->m_Outputs, &monitor->m_Link);

    wlr_output_layout_output* l_output = wlr_output_layout_add_auto(g_pCompositor->m_OutputLayout, output);
    wlr_scene_output* scene_output = wlr_scene_output_create(g_pCompositor->m_Scene, output);

    wlr_scene_output_layout_add_output(g_pCompositor->m_SceneLayout, l_output, scene_output);
}

void Events::HandleOutputDestroy(wl_listener* listener, void* data) {
    Monitor* monitor = wl_container_of(listener, monitor, m_Destroy);

    if (!monitor) {
        return;
    }

    Logger::Log(LogLevel::INFO, "--- Monitor Disconnected ---");

    wl_list_remove(&monitor->m_Frame.link);
    wl_list_remove(&monitor->m_RequestState.link);
    wl_list_remove(&monitor->m_Destroy.link);
    wl_list_remove(&monitor->m_Link);

    delete monitor;
}

void Events::HandleOutputRequestState(wl_listener* listener, void* data) {
    Monitor* monitor = wl_container_of(listener, monitor, m_RequestState);
    
    const wlr_output_event_request_state* event = static_cast<wlr_output_event_request_state*>(data);
    wlr_output_commit_state(monitor->m_WlrOutput, event->state);
}

void Events::HandleOutputFrame(wl_listener* listener, void* data){
    Monitor* monitor = wl_container_of(listener, monitor, m_Frame);
    wlr_scene* scene = g_pCompositor->m_Scene;
    
    wlr_scene_output* scene_output = wlr_scene_get_scene_output(scene, monitor->m_WlrOutput);
    wlr_scene_output_commit(scene_output, nullptr);

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}