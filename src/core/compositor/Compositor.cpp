#include "Compositor.hpp"
#include "./managers/WindowManager.hpp"
#include "../../include/Defines.hpp"
#include "../../debug/Debug.hpp"
#include "../output/MonitorManager.hpp"
#include "../input/InputManager.hpp"

Compositor::Compositor() {
    m_Display = wl_display_create();
    m_Backend = wlr_backend_autocreate(wl_display_get_event_loop(m_Display), nullptr);
    m_Renderer = wlr_renderer_autocreate(m_Backend);

    wlr_renderer_init_wl_display(m_Renderer, m_Display);

    m_Allocator = wlr_allocator_autocreate(m_Backend, m_Renderer);
    m_Compositor = wlr_compositor_create(m_Display, 5, m_Renderer);
	m_SubCompositor = wlr_subcompositor_create(m_Display);
	m_DataDeviceManager = wlr_data_device_manager_create(m_Display);
    m_OutputLayout = wlr_output_layout_create(m_Display);

    m_XWayland = wlr_xwayland_create(m_Display, m_Compositor, true);

    if (!m_XWayland) {
        log_warn("XWayland initialization failed.");
    }

    m_Scene = wlr_scene_create();
    m_SceneLayout = wlr_scene_attach_output_layout(m_Scene, m_OutputLayout);

    m_XDGShell = wlr_xdg_shell_create(m_Display, 3);
    m_ConfigManager = std::make_shared<FeatherConfig::ConfigManager>();
}

Compositor::~Compositor() {
    if (!m_CleaningUp) {
        Cleanup();
    }
}

bool Compositor::Initialize() {
    if (!m_Backend || !m_Renderer || !m_Allocator) return false;

    m_MonitorManager.Initialize();

    m_WindowManager.Initialize();
    m_LayoutManager.Initialize();
    m_DecorationManager.Initialize();

    m_InputManager.Initialize();
    m_SeatManager.Initialize();
    m_KeyboardManager.Initialize();
    m_MouseManager.Initialize();

    const char* socket = wl_display_add_socket_auto(m_Display);
    if (!socket || !wlr_backend_start(m_Backend)) return false;

    setenv("WAYLAND_DISPLAY", socket, 1);

    log_info("========================================");
    log_info(" feather initialized");
    log_info(" socket: %s", socket);
    log_info("========================================");

    return true;
}

void Compositor::Run() {
    wl_display_run(m_Display);
}

void Compositor::Cleanup() {
    log_info("exiting feather...");
    m_CleaningUp = true;

    wl_display_destroy_clients(m_Display);

    m_DecorationManager.Cleanup();
    m_LayoutManager.Cleanup();
    m_WindowManager.Cleanup();

    m_MouseManager.Cleanup();
    m_KeyboardManager.Cleanup();
    m_SeatManager.Cleanup();
    m_InputManager.Cleanup();

    m_MonitorManager.Cleanup();

    wlr_scene_node_destroy(&m_Scene->tree.node);
    wlr_allocator_destroy(m_Allocator);
    wlr_renderer_destroy(m_Renderer);
    wlr_backend_destroy(m_Backend);
    wl_display_destroy(m_Display);
}