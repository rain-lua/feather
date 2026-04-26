#include "SeatManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

SeatManager::SeatManager() {

}

void SeatManager::Initialize() {
    m_Seat = wlr_seat_create(g_pCompositor->m_Display, "seat0");

    m_RequestCursor.notify = SeatManager::SeatRequestCursor;
    m_RequestSetSelection.notify = SeatManager::SeatRequestSetSelection;
    m_PointerFocusChange.notify  = SeatManager::SeatPointerFocusChange;

    wl_signal_add(&g_pCompositor->m_SeatManager.m_Seat->events.request_set_cursor, &m_RequestCursor);
    wl_signal_add(&g_pCompositor->m_SeatManager.m_Seat->pointer_state.events.focus_change, &m_PointerFocusChange);
    wl_signal_add(&g_pCompositor->m_SeatManager.m_Seat->events.request_set_selection, &m_RequestSetSelection);
}

void SeatManager::Cleanup() {
    wl_list_remove(&m_RequestCursor.link);
    wl_list_remove(&m_PointerFocusChange.link);
    wl_list_remove(&m_RequestSetSelection.link);
}

void SeatManager::SeatRequestCursor(wl_listener* listener, void* data) {
	wlr_seat_pointer_request_set_cursor_event* event = static_cast<wlr_seat_pointer_request_set_cursor_event *>(data);
	wlr_seat_client* focused_client = g_pCompositor->m_SeatManager.m_Seat->pointer_state.focused_client;

	if (focused_client == event->seat_client) {
		wlr_cursor_set_surface(g_pCompositor->m_MouseManager.m_Cursor, event->surface, event->hotspot_x, event->hotspot_y);
	}
}

void SeatManager::SeatPointerFocusChange(wl_listener* listener, void* data) {
	wlr_seat_pointer_focus_change_event* event = static_cast<wlr_seat_pointer_focus_change_event *>(data);

	if (event->new_surface == nullptr) {
		wlr_cursor_set_xcursor(g_pCompositor->m_MouseManager.m_Cursor, g_pCompositor->m_MouseManager.m_XCursorManager, "default");
	}
}

void SeatManager::SeatRequestSetSelection(wl_listener* listener, void* data) {
	wlr_seat_request_set_selection_event* event = static_cast<wlr_seat_request_set_selection_event *>(data);
	wlr_seat_set_selection(g_pCompositor->m_SeatManager.m_Seat, event->source, event->serial);
}