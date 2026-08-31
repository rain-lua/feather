#include "Compositor.hpp"

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
    wlr_cursor_attach_output_layout(m_Cursor, m_OutputLayout);

    m_XCursorManager = wlr_xcursor_manager_create(nullptr, 24);
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
    m_InputManager      = std::make_unique<InputManager>();
    m_LayoutManager     = std::make_unique<LayoutManager>();

    wl_list_init(&m_Outputs);
    wl_list_init(&m_Windows);
    wl_list_init(&m_Pointers);
    wl_list_init(&m_Keyboards);

    m_CursorMode = CURSOR_PASSTHROUGH;

    wl_signal_add(&m_Backend->events.new_output, &m_NewOutput);
    wl_signal_add(&m_XDGShell->events.new_toplevel, &m_NewWindow);
    wl_signal_add(&g_pCompositor->m_Backend->events.new_input, &m_NewInput);

    wl_signal_add(&g_pCompositor->m_Cursor->events.motion, &m_CursorMotion);
    wl_signal_add(&g_pCompositor->m_Cursor->events.motion_absolute, &m_CursorMotionAbsolute);
    wl_signal_add(&g_pCompositor->m_Cursor->events.button, &m_CursorButton);
    wl_signal_add(&g_pCompositor->m_Cursor->events.axis, &m_CursorAxis);
    wl_signal_add(&g_pCompositor->m_Cursor->events.frame, &m_CursorFrame);

    wl_signal_add(&m_Seat->events.request_set_cursor, &m_RequestCursor);
    wl_signal_add(&m_Seat->pointer_state.events.focus_change, &m_PointerFocusChange);
    wl_signal_add(&m_Seat->events.request_set_selection, &m_RequestSetSelection);

    m_NewOutput.notify            = Events::HandleNewOutput;
    m_NewWindow.notify            = Events::HandleNewWindow;

    m_NewInput.notify             = Events::HandleNewInput;

    m_CursorMotion.notify         = Events::HandleCursorMotion;
    m_CursorMotionAbsolute.notify = Events::HandleCursorMotionAbsolute;
    m_CursorButton.notify         = Events::HandleCursorButton;
    m_CursorAxis.notify           = Events::HandleCursorAxis;
    m_CursorFrame.notify          = Events::HandleCursorFrame;

    m_RequestCursor.notify        = Events::SeatRequestCursor;
    m_RequestSetSelection.notify  = Events::SeatRequestSetSelection;
    m_PointerFocusChange.notify   = Events::SeatPointerFocusChange;

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

    wl_list_remove(&m_RequestCursor.link);
    wl_list_remove(&m_PointerFocusChange.link);
    wl_list_remove(&m_RequestSetSelection.link);

    wl_list_remove(&m_CursorMotion.link);
    wl_list_remove(&m_CursorMotionAbsolute.link);
    wl_list_remove(&m_CursorButton.link);
    wl_list_remove(&m_CursorAxis.link);
    wl_list_remove(&m_CursorFrame.link);

    wl_list_remove(&m_NewInput.link);
    wl_list_remove(&m_NewWindow.link);
    wl_list_remove(&m_NewOutput.link);

    if (m_SigIntSource && m_SigTermSource) {
        wl_event_source_remove(m_SigIntSource);
        wl_event_source_remove(m_SigTermSource);
    }

    if (m_XWayland) {
        wlr_xwayland_destroy(m_XWayland);

        m_XWayland = nullptr;
    }

    m_LayoutManager     = nullptr;
    m_InputManager      = nullptr;
    m_ConfigManager     = nullptr;

    wlr_xcursor_manager_destroy(m_XCursorManager);
    wlr_cursor_destroy(m_Cursor);

    wlr_scene_node_destroy(&m_Scene->tree.node);
    wlr_allocator_destroy(m_Allocator);
    wlr_renderer_destroy(m_Renderer);
    wlr_backend_destroy(m_Backend);
    wl_display_destroy(m_Display);
}

Window* Compositor::FindWindowAt(double lx, double ly, wlr_surface** surface, double* sx, double* sy) {
	wlr_scene_node* node = wlr_scene_node_at( &g_pCompositor->m_Scene->tree.node, lx, ly, sx, sy);

	if (node == nullptr || node->type != WLR_SCENE_NODE_BUFFER) {
		return nullptr;
	}

	wlr_scene_buffer* scene_buffer = wlr_scene_buffer_from_node(node);
	wlr_scene_surface* scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);

	if (!scene_surface) {
		return nullptr;
	}

	*surface = scene_surface->surface;

	wlr_scene_tree* tree = node->parent;

	while (tree != nullptr && tree->node.data == nullptr) {
		tree = tree->node.parent;
	}

	return static_cast<Window*>(tree->node.data);
}

void Compositor::FocusWindow(Window* window) {
	if (window == nullptr) {
		return;
	}
	
	wlr_seat* seat = m_Seat;
	wlr_surface* prev_surface = seat->keyboard_state.focused_surface;
	wlr_surface* surface = window->m_XDGToplevel->base->surface;

	if (prev_surface == surface) {
		return;
	}

	if (prev_surface) {
		wlr_xdg_toplevel* prev_window = wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
		
		if (prev_window != nullptr) {
			wlr_xdg_toplevel_set_activated(prev_window, false);
		}
	}

	wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

	wlr_scene_node_raise_to_top(&window->m_SceneTree->node);

	wl_list_remove(&window->m_Link);
	wl_list_insert(&m_Windows, &window->m_Link);
	
	m_FocusedWindow = window;
	wlr_xdg_toplevel_set_activated(window->m_XDGToplevel, true);

	if (keyboard != nullptr) {
        wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
	}
}

void Compositor::CloseWindow(Window* window) {
	if (window == nullptr || window->m_XDGToplevel == nullptr) {
        return;
    }
    
    wlr_xdg_toplevel_send_close(window->m_XDGToplevel);
}