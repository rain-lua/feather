#include "Monitors.hpp"

#include "../../debug/Logger.hpp"
#include "../../feather/Feather.hpp"

void HandleNewOutput(wl_listener* listener, void* data) {
    wlr_output* output = static_cast<wlr_output*>(data);

    Logger::Log(LogLevel::INFO, "--- New Monitor Connected: %s ---", output->name);

    wlr_output_init_render(output, g_Feather->m_allocator, g_Feather->m_renderer);

    wlr_output_state state;

    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    const MonitorConfig* config = g_Feather->m_ConfigManager->GetMonitorConfig(output->name);

    wlr_output_mode*     mode   = nullptr;

    if (!config) {
        Logger::Log(LogLevel::WARN, "No monitor config found for %s, using preferred mode", output->name);
    } else {
        Logger::Log(LogLevel::INFO, "Found config for %s: %dx%d@%.3lf", config->name.c_str(), config->width, config->height, config->refresh);

        wlr_output_mode* candidate;

        wl_list_for_each(candidate, &output->modes, link) {
            if (candidate->width != config->width) {
                continue;
            }

            if (candidate->height != config->height) {
                continue;
            }

            double candidateRefresh = candidate->refresh / 1000.0;
            double requestedRefresh = static_cast<double>(config->refresh);

            if (std::abs(candidateRefresh - requestedRefresh) <= 0.1) {
                mode = candidate;
                break;
            }
        }

        if (!mode) {
            Logger::Log(LogLevel::WARN,
                        "Requested mode %dx%d@%.3lf is unavailable on %s, "
                        "using preferred mode",
                        config->width, config->height, config->refresh, output->name);
        }
    }

    if (!mode) {
        mode = wlr_output_preferred_mode(output);
    }

    if (mode) {
        Logger::Log(LogLevel::INFO, "Using mode %dx%d@%.3lfHz for %s", mode->width, mode->height, mode->refresh / 1000.0, output->name);

        wlr_output_state_set_mode(&state, mode);
    } else {
        Logger::Log(LogLevel::WARN, "No output mode available for %s", output->name);
    }

    wlr_output_commit_state(output, &state);
    wlr_output_state_finish(&state);

    Monitor* monitor  = new Monitor;

    monitor->m_output = output;

    monitor->m_frame.Init(&output->events.frame, monitor, HandleOutputFrame);
    monitor->m_requestState.Init(&output->events.request_state, monitor, HandleOutputRequestState);
    monitor->m_destroy.Init(&output->events.destroy, monitor, HandleOutputDestroy);

    wl_list_insert(&g_Feather->m_outputs, &monitor->m_link);

    wlr_output_layout_output* lOutput     = wlr_output_layout_add_auto(g_Feather->m_outputLayout, output);
    wlr_scene_output*         sceneOutput = wlr_scene_output_create(g_Feather->m_scene, output);

    wlr_scene_output_layout_add_output(g_Feather->m_sceneLayout, lOutput, sceneOutput);
}

void HandleOutputDestroy(void* owner, void* data) {
    Monitor* monitor = static_cast<Monitor*>(owner);

    if (!monitor) {
        return;
    }

    Logger::Log(LogLevel::INFO, "--- Monitor Disconnected ---");

    monitor->m_frame.Remove();
    monitor->m_requestState.Remove();
    monitor->m_destroy.Remove();

    delete monitor;
}

void HandleOutputRequestState(void* owner, void* data) {
    Monitor*                              monitor = static_cast<Monitor*>(owner);

    const wlr_output_event_request_state* event   = static_cast<wlr_output_event_request_state*>(data);

    wlr_output_commit_state(monitor->m_output, event->state);
}

void HandleOutputFrame(void* owner, void* data) {
    Monitor*          monitor     = static_cast<Monitor*>(owner);

    wlr_scene*        scene       = g_Feather->m_scene;
    wlr_scene_output* sceneOutput = wlr_scene_get_scene_output(scene, monitor->m_output);

    wlr_scene_output_commit(sceneOutput, nullptr);

    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    wlr_scene_output_send_frame_done(sceneOutput, &now);
}