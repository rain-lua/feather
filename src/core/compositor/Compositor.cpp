#include "Compositor.hpp"
#include "./managers/WindowManager.hpp"
#include "../../include/Defines.hpp"
#include "../../debug/Logger.hpp"
#include "../output/MonitorManager.hpp"
#include "../input/InputManager.hpp"

#include <signal.h>

static int HandleSignal(int sig, void* data) {
    Logger::Log(LogLevel::INFO, "Received signal %d (%s)", sig, strsignal(sig));

    if (sig == SIGINT || sig == SIGTERM) {
        g_pCompositor->Stop();
    }

    return 0;
}

Compositor::Compositor() {
    m_Display = wl_display_create();

    if (!m_Display) {
        throw std::runtime_error("Failed to create display!");
    }

    m_EventLoop = wl_display_get_event_loop(m_Display);

    m_Backend = wlr_backend_autocreate(m_EventLoop, nullptr);

    if (!m_Backend) {
        throw std::runtime_error("Failed to create backend!");
    }

    m_Renderer = wlr_renderer_autocreate(m_Backend);

    if (!m_Renderer) {
        throw std::runtime_error("Failed to create renderer!");
    }

    wlr_renderer_init_wl_display(m_Renderer, m_Display);

    m_SigIntSource = wl_event_loop_add_signal(m_EventLoop, SIGINT, HandleSignal, nullptr);
    m_SigTermSource = wl_event_loop_add_signal(m_EventLoop, SIGTERM, HandleSignal, nullptr);

    m_Allocator = wlr_allocator_autocreate(m_Backend, m_Renderer);
    m_Compositor = wlr_compositor_create(m_Display, 5, m_Renderer);
	m_SubCompositor = wlr_subcompositor_create(m_Display);
	m_DataDeviceManager = wlr_data_device_manager_create(m_Display);
    m_OutputLayout = wlr_output_layout_create(m_Display);

    m_XWayland = wlr_xwayland_create(m_Display, m_Compositor, true);

    if (!m_Allocator) {
        throw std::runtime_error("Failed to create allocator!");
    }

    if (!m_XWayland) {
        Logger::Log(LogLevel::WARN, "Failed to create XWayland server!");
    }

    m_Scene = wlr_scene_create();
    m_SceneLayout = wlr_scene_attach_output_layout(m_Scene, m_OutputLayout);

    m_XDGShell = wlr_xdg_shell_create(m_Display, 3);

    m_Cursor = wlr_cursor_create();
    m_XCursorManager = wlr_xcursor_manager_create(nullptr, 24);

    wlr_cursor_attach_output_layout(m_Cursor, m_OutputLayout);
    wlr_xcursor_manager_load(m_XCursorManager, 1);

	m_Seat = wlr_seat_create(m_Display, "seat0");

    m_XDGDecorationManager = wlr_xdg_decoration_manager_v1_create(m_Display);
}

Compositor::~Compositor() {
    if (!m_CleaningUp) {
        Cleanup();
    }
}

bool Compositor::Initialize() {
    m_ConfigManager     = std::make_unique<ConfigManager>();
    m_MonitorManager    = std::make_unique<MonitorManager>();
    m_InputManager      = std::make_unique<InputManager>();
    m_SeatManager       = std::make_unique<SeatManager>();
    m_KeyboardManager   = std::make_unique<KeyboardManager>();
    m_MouseManager      = std::make_unique<MouseManager>();
    m_WindowManager     = std::make_unique<WindowManager>();
    m_LayoutManager     = std::make_unique<LayoutManager>();
    m_DecorationManager = std::make_unique<DecorationManager>();

    const char* socket = wl_display_add_socket_auto(m_Display);

    if (!socket) {
        Logger::Log(LogLevel::CRITICAL, "Failed to ensure wayland display socket!");

        wlr_backend_destroy(m_Backend);

        return false;
    }

    setenv("XDG_CURRENT_DESKTOP", "feather", 1);
    setenv("WAYLAND_DISPLAY", socket, 1);

    if (m_XWayland) {
        setenv("DISPLAY", m_XWayland->display_name, 1);
    }

    if (!wlr_backend_start(m_Backend)) {
        Logger::Log(LogLevel::CRITICAL, "Failed to start backend!");

        wlr_backend_destroy(m_Backend);
		wl_display_destroy(m_Display);

        return false;
    }

    Logger::Log(LogLevel::INFO, "========================================");
    Logger::Log(LogLevel::INFO, " Feather initialized!");
    Logger::Log(LogLevel::INFO, " socket: %s", socket);
    Logger::Log(LogLevel::INFO, "========================================");

    return true;
}

void Compositor::Run() {
    Logger::Log(LogLevel::INFO, "Running Feather...");

    wl_display_run(m_Display);
}

void Compositor::Stop() {
    Logger::Log(LogLevel::INFO, "Stopping Feather...");

    wl_display_terminate(m_Display);
}

void Compositor::Cleanup() {
    Logger::Log(LogLevel::INFO, "Exiting Feather...");

    if (!m_Display) {
        return;
    }

    m_CleaningUp = true;

    wl_display_destroy_clients(m_Display);

    m_DecorationManager = nullptr;
    m_LayoutManager     = nullptr;
    m_WindowManager     = nullptr;
    m_MouseManager      = nullptr;
    m_KeyboardManager   = nullptr;
    m_SeatManager       = nullptr;
    m_InputManager      = nullptr;
    m_MonitorManager    = nullptr;
    m_ConfigManager     = nullptr;

    if (m_SigIntSource && m_SigTermSource) {
        wl_event_source_remove(m_SigIntSource);
        wl_event_source_remove(m_SigTermSource);
    }

    if (m_XWayland) {
        wlr_xwayland_destroy(m_XWayland);

        m_XWayland = nullptr;
    }

    wlr_xcursor_manager_destroy(m_XCursorManager);
    wlr_cursor_destroy(m_Cursor);

    wlr_scene_node_destroy(&m_Scene->tree.node);
    wlr_allocator_destroy(m_Allocator);
    wlr_renderer_destroy(m_Renderer);
    wlr_backend_destroy(m_Backend);
    wl_display_destroy(m_Display);
}