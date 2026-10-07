#include "Feather.hpp"

#include "../debug/Logger.hpp"

#include "./hooks/Devices.hpp"
#include "./hooks/Monitors.hpp"
#include "./hooks/Windows.hpp"

#include "./misc/Misc.hpp"

#include <signal.h>
#include <stdexcept>

static int HandleSignal(int sig, void* data) {
    Logger::Log(LogLevel::INFO, "Received signal %d (%s)", sig, strsignal(sig));

    if (sig == SIGINT || sig == SIGTERM) {
        g_Feather->Stop();
    }

    return 0;
}

Feather::Feather() {
    m_display = wl_display_create();

    if (!m_display) {
        throw std::runtime_error("Failed to create display!");
    }

    m_eventLoop = wl_display_get_event_loop(m_display);
    m_backend   = wlr_backend_autocreate(m_eventLoop, nullptr);

    if (!m_backend) {
        throw std::runtime_error("Failed to create backend!");
    }

    m_renderer = wlr_renderer_autocreate(m_backend);

    if (!m_renderer) {
        throw std::runtime_error("Failed to create renderer!");
    }

    wlr_renderer_init_wl_display(m_renderer, m_display);

    m_sigIntSource      = wl_event_loop_add_signal(m_eventLoop, SIGINT, HandleSignal, nullptr);
    m_sigTermSource     = wl_event_loop_add_signal(m_eventLoop, SIGTERM, HandleSignal, nullptr);

    m_allocator         = wlr_allocator_autocreate(m_backend, m_renderer);
    m_compositor        = wlr_compositor_create(m_display, 5, m_renderer);
    m_subcompositor     = wlr_subcompositor_create(m_display);
    m_dataDeviceManager = wlr_data_device_manager_create(m_display);
    m_outputLayout      = wlr_output_layout_create(m_display);

    if (!m_allocator) {
        throw std::runtime_error("Failed to create allocator!");
    }

    CreateXWayland();

    m_scene       = wlr_scene_create();
    m_sceneLayout = wlr_scene_attach_output_layout(m_scene, m_outputLayout);

    m_xdgShell    = wlr_xdg_shell_create(m_display, 3);

    m_cursor      = wlr_cursor_create();
    wlr_cursor_attach_output_layout(m_cursor, m_outputLayout);

    m_xcursorManager = wlr_xcursor_manager_create(nullptr, 24);
    wlr_xcursor_manager_load(m_xcursorManager, 1);

    m_seat                 = wlr_seat_create(m_display, "seat0");

    m_xdgDecorationManager = wlr_xdg_decoration_manager_v1_create(m_display);
}

Feather::~Feather() {
    if (!m_cleaningUp) {
        Cleanup();
    }
}

bool Feather::Initialize() {
    m_ConfigManager = std::make_unique<ConfigManager>();
    m_InputHandler  = std::make_unique<InputHandler>();
    m_LayoutManager = std::make_unique<LayoutManager>();

    wl_list_init(&m_outputs);
    wl_list_init(&m_windows);
    wl_list_init(&m_pointers);
    wl_list_init(&m_keyboards);

    m_cursorMode = CURSOR_PASSTHROUGH;

    AddSignal(&m_backend->events.new_output, &m_newOutput, HandleNewOutput);
    AddSignal(&m_xdgShell->events.new_toplevel, &m_newWindow, HandleNewWindow);
    AddSignal(&m_backend->events.new_input, &m_newInput, HandleNewInput);

    AddSignal(&m_cursor->events.motion, &m_cursorMotion, HandleCursorMotion);
    AddSignal(&m_cursor->events.motion_absolute, &m_cursorMotionAbsolute, HandleCursorMotionAbsolute);
    AddSignal(&m_cursor->events.button, &m_cursorButton, HandleCursorButton);
    AddSignal(&m_cursor->events.axis, &m_cursorAxis, HandleCursorAxis);
    AddSignal(&m_cursor->events.frame, &m_cursorFrame, HandleCursorFrame);

    AddSignal(&m_seat->events.request_set_cursor, &m_requestCursor, SeatRequestCursor);
    AddSignal(&m_seat->pointer_state.events.focus_change, &m_pointerFocusChange, SeatPointerFocusChange);
    AddSignal(&m_seat->events.request_set_selection, &m_requestSetSelection, SeatRequestSetSelection);

    const char* socket = wl_display_add_socket_auto(m_display);

    if (!socket) {
        Logger::Log(LogLevel::CRITICAL, "Failed to ensure wayland display socket!");

        wlr_backend_destroy(m_backend);

        return false;
    }

    setenv("XDG_CURRENT_DESKTOP", "feather", 1);
    setenv("WAYLAND_DISPLAY", socket, 1);

    if (m_xwayland) {
        setenv("DISPLAY", m_xwayland->display_name, 1);
    }

    if (!wlr_backend_start(m_backend)) {
        Logger::Log(LogLevel::CRITICAL, "Failed to start backend!");

        wlr_backend_destroy(m_backend);
        wl_display_destroy(m_display);

        return false;
    }

    m_ConfigManager->RunStartupExecs();

    Logger::Log(LogLevel::INFO, "========================================");
    Logger::Log(LogLevel::INFO, " Feather initialized!");
    Logger::Log(LogLevel::INFO, " socket: %s", socket);
    Logger::Log(LogLevel::INFO, "========================================");

    return true;
}

void Feather::Run() {
    Logger::Log(LogLevel::INFO, "Running Feather...");

    wl_display_run(m_display);
}

void Feather::Stop() {
    Logger::Log(LogLevel::INFO, "Stopping Feather...");

    wl_display_terminate(m_display);
}

void Feather::Cleanup() {
    Logger::Log(LogLevel::INFO, "Exiting Feather...");

    if (!m_display) {
        return;
    }

    m_cleaningUp = true;

    wl_display_destroy_clients(m_display);

    wl_list_remove(&m_requestCursor.link);
    wl_list_remove(&m_pointerFocusChange.link);
    wl_list_remove(&m_requestSetSelection.link);

    wl_list_remove(&m_cursorMotion.link);
    wl_list_remove(&m_cursorMotionAbsolute.link);
    wl_list_remove(&m_cursorButton.link);
    wl_list_remove(&m_cursorAxis.link);
    wl_list_remove(&m_cursorFrame.link);

    wl_list_remove(&m_newInput.link);
    wl_list_remove(&m_newWindow.link);
    wl_list_remove(&m_newOutput.link);

    if (m_sigIntSource && m_sigTermSource) {
        wl_event_source_remove(m_sigIntSource);
        wl_event_source_remove(m_sigTermSource);
    }

    if (m_xwayland) {
        wlr_xwayland_destroy(m_xwayland);

        m_xwayland = nullptr;
    }

    m_LayoutManager = nullptr;
    m_InputHandler  = nullptr;
    m_ConfigManager = nullptr;

    wlr_xcursor_manager_destroy(m_xcursorManager);
    wlr_cursor_destroy(m_cursor);

    wlr_scene_node_destroy(&m_scene->tree.node);
    wlr_allocator_destroy(m_allocator);
    wlr_renderer_destroy(m_renderer);
    wlr_backend_destroy(m_backend);
    wl_display_destroy(m_display);
}

void Feather::CreateXWayland() {
#ifdef XWAYLAND
    m_xwayland = wlr_xwayland_create(m_display, m_compositor, true);

    if (!m_xwayland) {
        Logger::Log(LogLevel::WARN, "Failed to create XWayland server!");
    }
#endif
}

Window* Feather::FindWindowAt(double lx, double ly, wlr_surface** surface, double* sx, double* sy) {
    wlr_scene_node* node = wlr_scene_node_at(&m_scene->tree.node, lx, ly, sx, sy);

    if (node == nullptr || node->type != WLR_SCENE_NODE_BUFFER) {
        return nullptr;
    }

    wlr_scene_buffer*  sceneBuffer  = wlr_scene_buffer_from_node(node);
    wlr_scene_surface* sceneSurface = wlr_scene_surface_try_from_buffer(sceneBuffer);

    if (!sceneSurface) {
        return nullptr;
    }

    *surface             = sceneSurface->surface;

    wlr_scene_tree* tree = node->parent;

    while (tree != nullptr && tree->node.data == nullptr) {
        tree = tree->node.parent;
    }

    return static_cast<Window*>(tree->node.data);
}

void Feather::FocusWindow(Window* window) {
    if (window == nullptr) {
        return;
    }

    wlr_seat*    seat        = m_seat;
    wlr_surface* prevSurface = seat->keyboard_state.focused_surface;
    wlr_surface* surface     = window->m_xdgToplevel->base->surface;

    if (prevSurface == surface) {
        return;
    }

    if (prevSurface) {
        wlr_xdg_toplevel* prevWindow = wlr_xdg_toplevel_try_from_wlr_surface(prevSurface);

        if (prevWindow != nullptr) {
            wlr_xdg_toplevel_set_activated(prevWindow, false);
        }
    }

    wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

    wlr_scene_node_raise_to_top(&window->m_sceneTree->node);

    wl_list_remove(&window->m_link);
    wl_list_insert(&m_windows, &window->m_link);

    m_focusedWindow = window;
    wlr_xdg_toplevel_set_activated(window->m_xdgToplevel, true);

    if (keyboard != nullptr) {
        wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }
}

void Feather::CloseWindow(Window* window) {
    if (window == nullptr || window->m_xdgToplevel == nullptr) {
        return;
    }

    wlr_xdg_toplevel_send_close(window->m_xdgToplevel);
}