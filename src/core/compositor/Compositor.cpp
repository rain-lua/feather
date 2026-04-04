#include "Compositor.hpp"
#include "../../include/Defines.hpp"
#include "../../debug/Debug.hpp"
#include "../monitor/Monitor.hpp"
#include "../input/InputManager.hpp"

Compositor::Compositor() : 
    m_Display(nullptr), m_Backend(nullptr), m_Renderer(nullptr), 
    m_Allocator(nullptr), m_Scene(nullptr), m_SceneLayout(nullptr), 
    m_OutputLayout(nullptr), m_Seat(nullptr) {

    m_Display = wl_display_create();
    m_Backend = wlr_backend_autocreate(wl_display_get_event_loop(m_Display), nullptr);
    m_Renderer = wlr_renderer_autocreate(m_Backend);

    wlr_renderer_init_wl_display(m_Renderer, m_Display);

    m_Allocator = wlr_allocator_autocreate(m_Backend, m_Renderer);
    m_OutputLayout = wlr_output_layout_create(m_Display);

    wl_list_init(&m_Outputs);
    wl_list_init(&m_Keyboards);

    m_Scene = wlr_scene_create();
    m_SceneLayout = wlr_scene_attach_output_layout(m_Scene, m_OutputLayout);
    m_Seat = wlr_seat_create(m_Display, "seat0");
}

bool Compositor::Initialize() {
    if (!m_Backend || !m_Renderer || !m_Allocator) return false;

    m_NewOutput.notify = MonitorManager::HandleNewOutput;
    wl_signal_add(&m_Backend->events.new_output, &m_NewOutput);

    m_NewInput.notify = InputManager::HandleNewInput;
    wl_signal_add(&m_Backend->events.new_input, &m_NewInput);

    const char *socket = wl_display_add_socket_auto(m_Display);
    if (!socket || !wlr_backend_start(m_Backend)) return false;

    setenv("WAYLAND_DISPLAY", socket, true);

    log_info("========================================");
    log_info(" fwm initialized");
    log_info(" socket: %s", socket);
    log_info("========================================");

    return true;
}

void Compositor::Run() {
    wl_display_run(m_Display);
}

void Compositor::Cleanup() {
    log_info("exiting fwm...");

    wl_display_destroy_clients(m_Display);
    wl_list_remove(&m_NewInput.link);
    wl_list_remove(&m_NewOutput.link);

    wlr_backend_destroy(m_Backend);
    wlr_allocator_destroy(m_Allocator);
    wlr_renderer_destroy(m_Renderer);
    wl_display_destroy(m_Display);
}