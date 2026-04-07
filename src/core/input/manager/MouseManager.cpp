#include "MouseManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

void MouseManager::HandleNewPointer(Compositor *server, wlr_input_device *device) {
    wlr_cursor_attach_input_device(server->m_Cursor, device);

	server->m_CursorMode = CURSOR_PASSTHROUGH;

	server->m_CursorMotion.notify = MouseManager::HandleCursorMotion;
	server->m_CursorMotionAbsolute.notify = MouseManager::HandleCursorMotionAbsolute;
	server->m_CursorButton.notify = MouseManager::HandleCursorButton;
	server->m_CursorAxis.notify = MouseManager::HandleCursorAxis;
	server->m_CursorFrame.notify = MouseManager::HandleCursorFrame;

	server->m_RequestCursor.notify = MouseManager::SeatRequestCursor;
	server->m_RequestSetSelection.notify = MouseManager::SeatRequestSetSelection;
	server->m_PointerFocusChange.notify = MouseManager::SeatPointerFocusChange;

	wl_signal_add(&server->m_Cursor->events.motion, &server->m_CursorMotion);
	wl_signal_add(&server->m_Cursor->events.motion_absolute, &server->m_CursorMotionAbsolute);
	wl_signal_add(&server->m_Cursor->events.button, &server->m_CursorButton);
	wl_signal_add(&server->m_Cursor->events.axis, &server->m_CursorAxis);
	wl_signal_add(&server->m_Cursor->events.frame, &server->m_CursorFrame);

	wl_signal_add(&server->m_Seat->events.request_set_cursor, &server->m_RequestCursor);
	wl_signal_add(&server->m_Seat->pointer_state.events.focus_change, &server->m_PointerFocusChange);
	wl_signal_add(&server->m_Seat->events.request_set_selection, &server->m_RequestSetSelection);
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

void MouseManager::SeatPointerFocusChange(struct wl_listener *listener, void *data) {
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
    server->m_CursorMode= CURSOR_PASSTHROUGH;
}

void MouseManager::ProcessCursorMotion(Compositor *server, uint32_t time) {
    //basic implementation rn
    wlr_cursor_set_xcursor(server->m_Cursor, server->m_CursorManager, "default");
}

void MouseManager::HandleCursorMotion(wl_listener *listener, void *data) {
	log_debug("Move cursor");
    Compositor *server = wl_container_of(listener, server, m_CursorMotion);
	wlr_pointer_motion_event *event = static_cast<wlr_pointer_motion_event *>(data);

	wlr_cursor_move(server->m_Cursor, &event->pointer->base, event->delta_x, event->delta_y);
	MouseManager::ProcessCursorMotion(server, event->time_msec);
}

void MouseManager::HandleCursorMotionAbsolute(wl_listener *listener, void *data){
    Compositor *server = wl_container_of(listener, server, m_CursorMotionAbsolute);
	struct wlr_pointer_motion_absolute_event *event = static_cast<wlr_pointer_motion_absolute_event *>(data);
	wlr_cursor_warp_absolute(server->m_Cursor, &event->pointer->base, event->x, event->y);
	MouseManager::ProcessCursorMotion(server, event->time_msec);
}

void MouseManager::HandleCursorButton(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorButton);
	struct wlr_pointer_button_event *event = static_cast<wlr_pointer_button_event *>(data);

	wlr_seat_pointer_notify_button(server->m_Seat, event->time_msec, event->button, event->state);

	if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
		MouseManager::ResetCursorMode(server);
	} else {
        //window grabbing focus stuff
	}
}

void MouseManager::HandleCursorAxis(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorAxis);
	struct wlr_pointer_axis_event *event = static_cast<wlr_pointer_axis_event *>(data);
	wlr_seat_pointer_notify_axis(server->m_Seat, event->time_msec, event->orientation, event->delta, event->delta_discrete, event->source, event->relative_direction);
}

void MouseManager::HandleCursorFrame(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_CursorFrame);
	wlr_seat_pointer_notify_frame(server->m_Seat);
}