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

    wl_list_init(&m_Outputs);
    wl_list_init(&m_Keyboards);
    wl_list_init(&m_Pointers);
    wl_list_init(&m_Windows);

    m_RequestCursor.notify = MouseManager::SeatRequestCursor;
	m_RequestSetSelection.notify = MouseManager::SeatRequestSetSelection;
	m_PointerFocusChange.notify = MouseManager::SeatPointerFocusChange;

    m_Cursor = wlr_cursor_create();
    m_FocusedWindow = nullptr;

    wlr_cursor_attach_output_layout(m_Cursor, m_OutputLayout);

    const char* theme = getenv("XCURSOR_THEME");
    if (!theme) theme = "default";

    const char* sizeStr = getenv("XCURSOR_SIZE");
    int size = sizeStr ? atoi(sizeStr) : 24;

    m_CursorManager = wlr_xcursor_manager_create(theme, size);

    m_Scene = wlr_scene_create();
    m_SceneLayout = wlr_scene_attach_output_layout(m_Scene, m_OutputLayout);
    m_Seat = wlr_seat_create(m_Display, "seat0");

    m_DecorationManager = wlr_xdg_decoration_manager_v1_create(m_Display);
    m_XDGShell = wlr_xdg_shell_create(m_Display, 3);

    m_ConfigManager = std::make_shared<FeatherConfig::ConfigManager>();
}

bool Compositor::Initialize() {
    if (!m_Backend || !m_Renderer || !m_Allocator) return false;

    m_NewOutput.notify = MonitorManager::HandleNewOutput;
    wl_signal_add(&m_Backend->events.new_output, &m_NewOutput);

    m_NewInput.notify = InputManager::HandleNewInput;
    wl_signal_add(&m_Backend->events.new_input, &m_NewInput);

    m_NewWindow.notify = WindowManager::HandleNewWindow;
    wl_signal_add(&m_XDGShell->events.new_toplevel, &m_NewWindow);

    m_NewDecoration.notify = DecorationManager::HandleNewDecoration;
    wl_signal_add(&m_DecorationManager->events.new_toplevel_decoration, &m_NewDecoration);

    // well rn we will just handle shit there <3

    m_CursorMode = CURSOR_PASSTHROUGH;

	m_CursorMotion.notify = MouseManager::HandleCursorMotion;
	m_CursorMotionAbsolute.notify = MouseManager::HandleCursorMotionAbsolute;
	m_CursorButton.notify = MouseManager::HandleCursorButton;
	m_CursorAxis.notify = MouseManager::HandleCursorAxis;
	m_CursorFrame.notify = MouseManager::HandleCursorFrame;

	wl_signal_add(&m_Cursor->events.motion, &m_CursorMotion);
	wl_signal_add(&m_Cursor->events.motion_absolute, &m_CursorMotionAbsolute);
	wl_signal_add(&m_Cursor->events.button, &m_CursorButton);
	wl_signal_add(&m_Cursor->events.axis, &m_CursorAxis);
	wl_signal_add(&m_Cursor->events.frame, &m_CursorFrame);

    wl_signal_add(&m_Seat->events.request_set_cursor, &m_RequestCursor);
	wl_signal_add(&m_Seat->pointer_state.events.focus_change, &m_PointerFocusChange);
	wl_signal_add(&m_Seat->events.request_set_selection, &m_RequestSetSelection);

    wlr_xcursor_manager_load(m_CursorManager, 1);

    const char *socket = wl_display_add_socket_auto(m_Display);
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

    wl_display_destroy_clients(m_Display);

    wl_list_remove(&m_NewWindow.link);
    wl_list_remove(&m_NewDecoration.link);

    // ~~for now we need this as we made these members of the compositor class, i think we will move these into the cursor struct itself soon~~
    // ~~m_Cursor and m_CursorManager will be kept global~~
    // Hi, past me, this is future me, this actually might not be the best idea, stated above.
    // It would be better to leave this global probably, theres a difference between a cursor (visual indicator) and a pointer (actual device)
    wl_list_remove(&m_CursorMotion.link);
    wl_list_remove(&m_CursorMotionAbsolute.link);
    wl_list_remove(&m_CursorButton.link);
    wl_list_remove(&m_CursorAxis.link);
    wl_list_remove(&m_CursorFrame.link);

    wl_list_remove(&m_NewInput.link);

    // this destroys seat-related listeners. maybe we will make a seat class in the future to handle this
    wl_list_remove(&m_RequestCursor.link);
    wl_list_remove(&m_PointerFocusChange.link);
    wl_list_remove(&m_RequestSetSelection.link);

    wl_list_remove(&m_NewOutput.link);

    // also we removed destroying the seat here for now. it was causing a segmentation fault
    // wlr_seat_destroy(m_Seat);

    wlr_scene_node_destroy(&m_Scene->tree.node);
    wlr_xcursor_manager_destroy(m_CursorManager);
    wlr_cursor_destroy(m_Cursor);
    wlr_allocator_destroy(m_Allocator);
    wlr_renderer_destroy(m_Renderer);
    wlr_backend_destroy(m_Backend);
    wl_display_destroy(m_Display);
}