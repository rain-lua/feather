#include "MouseManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

void MouseManager::HandleNewPointer(wlr_input_device *device) {
    wlr_cursor_attach_input_device(g_pCompositor->m_Cursor, device);

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
	wlr_seat_pointer_request_set_cursor_event *event = static_cast<wlr_seat_pointer_request_set_cursor_event *>(data);
	wlr_seat_client *focused_client = g_pCompositor->m_Seat->pointer_state.focused_client;

	if (focused_client == event->seat_client) {
		wlr_cursor_set_surface(g_pCompositor->m_Cursor, event->surface, event->hotspot_x, event->hotspot_y);
	}
}

void MouseManager::SeatPointerFocusChange(wl_listener *listener, void *data) {
	wlr_seat_pointer_focus_change_event *event = static_cast<wlr_seat_pointer_focus_change_event *>(data);

	if (event->new_surface == NULL) {
		wlr_cursor_set_xcursor(g_pCompositor->m_Cursor, g_pCompositor->m_CursorManager, "default");
	}
}

void MouseManager::SeatRequestSetSelection(wl_listener *listener, void *data) {
	wlr_seat_request_set_selection_event *event = static_cast<wlr_seat_request_set_selection_event *>(data);
	wlr_seat_set_selection(g_pCompositor->m_Seat, event->source, event->serial);
}

void MouseManager::ResetCursorMode() {
    g_pCompositor->m_CursorMode = CURSOR_PASSTHROUGH;
}

void MouseManager::ProcessCursorMotion(uint32_t time) {
    if (g_pCompositor->m_CursorMode == CURSOR_MOVE) {
		//stuff
        return;
    } else if (g_pCompositor->m_CursorMode == CURSOR_RESIZE) {
		//stuff
        return;
    }

    double sx, sy;
    wlr_seat *seat = g_pCompositor->m_Seat;
    wlr_surface *surface = NULL;

    wlr_scene_node *node = wlr_scene_node_at(&g_pCompositor->m_Scene->tree.node, g_pCompositor->m_Cursor->x, g_pCompositor->m_Cursor->y, &sx, &sy);

    if (!node) {
        wlr_cursor_set_xcursor(g_pCompositor->m_Cursor, g_pCompositor->m_CursorManager, "default");
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
	wlr_pointer_motion_event *event = static_cast<wlr_pointer_motion_event *>(data);

	wlr_cursor_move(g_pCompositor->m_Cursor, &event->pointer->base, event->delta_x, event->delta_y);
	MouseManager::ProcessCursorMotion(event->time_msec);
}

void MouseManager::HandleCursorMotionAbsolute(wl_listener *listener, void *data){
	wlr_pointer_motion_absolute_event *event = static_cast<wlr_pointer_motion_absolute_event *>(data);
	wlr_cursor_warp_absolute(g_pCompositor->m_Cursor, &event->pointer->base, event->x, event->y);
	MouseManager::ProcessCursorMotion(event->time_msec);
}

void MouseManager::HandleCursorButton(wl_listener *listener, void *data) {
	wlr_pointer_button_event *event = static_cast<wlr_pointer_button_event *>(data);

	wlr_seat_pointer_notify_button(g_pCompositor->m_Seat, event->time_msec, event->button, event->state);

	if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
		MouseManager::ResetCursorMode();
	} else {
        double sx;
        double sy;

		wlr_surface *surface = nullptr;

		Window *window = g_pCompositor->m_WindowManager.FindWindowAt(g_pCompositor->m_Cursor->x, g_pCompositor->m_Cursor->y, &surface, &sx, &sy);
		g_pCompositor->m_WindowManager.FocusWindow(window);
	}
}

void MouseManager::HandleCursorAxis(wl_listener *listener, void *data) {
	wlr_pointer_axis_event *event = static_cast<wlr_pointer_axis_event *>(data);
	wlr_seat_pointer_notify_axis(g_pCompositor->m_Seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source, event->relative_direction);
}

void MouseManager::HandleCursorFrame(wl_listener *listener, void *data) {
	wlr_seat_pointer_notify_frame(g_pCompositor->m_Seat);
}