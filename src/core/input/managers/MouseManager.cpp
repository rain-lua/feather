#include "MouseManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

void MouseManager::HandleNewPointer(Compositor *server, wlr_input_device *device) {
    wlr_cursor_attach_input_device(server->m_Cursor, device);

    // btw we removed shit there cuz to fix some assertion failing
    // prolly a better idea overall cuz it might act weird with multiple pointers
}

void MouseManager::HandlePointerDestroy(wl_listener *listener, void *data) {
    log_info("--- Pointer Disconnected ---");

    Pointer *pointer = wl_container_of(listener, pointer, m_Destroy);

    wl_list_remove(&pointer->m_Destroy.link);
    wl_list_remove(&pointer->m_Link);

    free(pointer);
}

void MouseManager::SeatRequestCursor(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_RequestCursor);
	wlr_seat_pointer_request_set_cursor_event *event = static_cast<wlr_seat_pointer_request_set_cursor_event *>(data);
	wlr_seat_client *focused_client = server->m_Seat->pointer_state.focused_client;

	if (focused_client == event->seat_client) {
		wlr_cursor_set_surface(server->m_Cursor, event->surface, event->hotspot_x, event->hotspot_y);
	}
}

void MouseManager::SeatPointerFocusChange(wl_listener *listener, void *data) {
	Compositor *server = wl_container_of(listener, server, m_PointerFocusChange);
	wlr_seat_pointer_focus_change_event *event = static_cast<wlr_seat_pointer_focus_change_event *>(data);

	if (event->new_surface == NULL) {
		wlr_cursor_set_xcursor(server->m_Cursor, server->m_CursorManager, "default");
	}
}

void MouseManager::SeatRequestSetSelection(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_RequestSetSelection);
	wlr_seat_request_set_selection_event *event = static_cast<wlr_seat_request_set_selection_event *>(data);
	wlr_seat_set_selection(server->m_Seat, event->source, event->serial);
}

void MouseManager::ResetCursorMode(Compositor *server) {
    server->m_CursorMode = CURSOR_PASSTHROUGH;
}

void MouseManager::ProcessCursorMotion(Compositor *server, uint32_t time) {
    if (server->m_CursorMode == CURSOR_MOVE) {
		//stuff
        return;
    } else if (server->m_CursorMode == CURSOR_RESIZE) {
		//stuff
        return;
    }

    double sx, sy;
    wlr_seat *seat = server->m_Seat;
    wlr_surface *surface = NULL;

    wlr_scene_node *node = wlr_scene_node_at(&server->m_Scene->tree.node, server->m_Cursor->x, server->m_Cursor->y, &sx, &sy);

    if (!node) {
        wlr_cursor_set_xcursor(server->m_Cursor, server->m_CursorManager, "default");
    }

    if (node && node->type == WLR_SCENE_NODE_BUFFER) {
        wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
        wlr_scene_surface *scene_surface = wlr_scene_surface_try_from_buffer(scene_buffer);
        
        if (scene_surface) {
            surface = scene_surface->surface;
        }
    }

    if (surface) {
        wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(seat, time, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(seat);
    }
}

void MouseManager::HandleCursorMotion(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorMotion);
	wlr_pointer_motion_event *event = static_cast<wlr_pointer_motion_event *>(data);

	wlr_cursor_move(server->m_Cursor, &event->pointer->base, event->delta_x, event->delta_y);
	MouseManager::ProcessCursorMotion(server, event->time_msec);
}

void MouseManager::HandleCursorMotionAbsolute(wl_listener *listener, void *data){
    Compositor *server = wl_container_of(listener, server, m_CursorMotionAbsolute);
	wlr_pointer_motion_absolute_event *event = static_cast<wlr_pointer_motion_absolute_event *>(data);
	wlr_cursor_warp_absolute(server->m_Cursor, &event->pointer->base, event->x, event->y);
	MouseManager::ProcessCursorMotion(server, event->time_msec);
}

void MouseManager::HandleCursorButton(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorButton);
	wlr_pointer_button_event *event = static_cast<wlr_pointer_button_event *>(data);

	wlr_seat_pointer_notify_button(server->m_Seat, event->time_msec, event->button, event->state);

	if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
		MouseManager::ResetCursorMode(server);
	} else {
        double sx;
        double sy;

		wlr_surface *surface = nullptr;

		Window *window = WindowManager::FindWindowAt(server, server->m_Cursor->x, server->m_Cursor->y, &surface, &sx, &sy);
		WindowManager::FocusWindow(window);
	}
}

void MouseManager::HandleCursorAxis(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorAxis);
	wlr_pointer_axis_event *event = static_cast<wlr_pointer_axis_event *>(data);
	wlr_seat_pointer_notify_axis(server->m_Seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source, event->relative_direction);
}

void MouseManager::HandleCursorFrame(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorFrame);
	wlr_seat_pointer_notify_frame(server->m_Seat);
}