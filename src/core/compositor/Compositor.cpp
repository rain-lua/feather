#include "Compositor.hpp"
#include "WindowManager.hpp"
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
    wlr_compositor_create(m_Display, 5, m_Renderer);
	wlr_subcompositor_create(m_Display);
	wlr_data_device_manager_create(m_Display);
    m_OutputLayout = wlr_output_layout_create(m_Display);

    wl_list_init(&m_Outputs);
    wl_list_init(&m_Keyboards);
    wl_list_init(&m_Pointers);
    wl_list_init(&m_Windows);

    m_Cursor = wlr_cursor_create();

    wlr_cursor_attach_output_layout(m_Cursor, m_OutputLayout);

    const char* theme = getenv("XCURSOR_THEME");
    if (!theme) theme = "default";

    const char* sizeStr = getenv("XCURSOR_SIZE");
    int size = sizeStr ? atoi(sizeStr) : 24;

    m_CursorManager = wlr_xcursor_manager_create(theme, size);

    m_Scene = wlr_scene_create();
    m_SceneLayout = wlr_scene_attach_output_layout(m_Scene, m_OutputLayout);
    m_Seat = wlr_seat_create(m_Display, "seat0");

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

    wlr_xcursor_manager_load(m_CursorManager, 1);

    const char *socket = wl_display_add_socket_auto(m_Display);
    if (!socket || !wlr_backend_start(m_Backend)) return false;

    setenv("WAYLAND_DISPLAY", socket, true);

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
    wl_list_remove(&m_NewInput.link);
    wl_list_remove(&m_NewOutput.link);

    //for now we need this as we made these members of the compositor class, i think we will move these into the cursor struct itself soon
    // m_Cursor and m_CursorManager will be kept global
    wl_list_remove(&m_CursorMotion.link);
    wl_list_remove(&m_CursorMotionAbsolute.link);
    wl_list_remove(&m_CursorButton.link);
    wl_list_remove(&m_CursorAxis.link);
    wl_list_remove(&m_CursorFrame.link);

    // this destroys seat-related listeners. maybe we will make a seat class in the future to handle this
    wl_list_remove(&m_RequestCursor.link);
    wl_list_remove(&m_PointerFocusChange.link);
    wl_list_remove(&m_RequestSetSelection.link);

    wlr_seat_destroy(m_Seat);
    wlr_cursor_destroy(m_Cursor);
    wlr_xcursor_manager_destroy(m_CursorManager);
    wlr_scene_node_destroy(&m_Scene->tree.node);
    wlr_allocator_destroy(m_Allocator);
    wlr_renderer_destroy(m_Renderer);
    wlr_backend_destroy(m_Backend);
    wl_display_destroy(m_Display);
}