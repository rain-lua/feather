#include "Events.hpp"


void Events::HandleNewInput(wl_listener* listener, void* data) {
    wlr_input_device* device = static_cast<wlr_input_device*>(data);

    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            Logger::Log(LogLevel::INFO, "--- New Keyboard Connected: %s ---", device->name);
            g_pCompositor->m_InputManager->HandleNewKeyboard(device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            Logger::Log(LogLevel::INFO, "--- New Pointer Connected: %s ---", device->name);
            g_pCompositor->m_InputManager->HandleNewPointer(device);
            break;
        default:
            Logger::Log(LogLevel::INFO, "--- New Input Device (%d): %s ---", device->type, device->name);
            break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    
    if (!wl_list_empty(&g_pCompositor->m_Keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    
    wlr_seat_set_capabilities(g_pCompositor->m_Seat, caps);
}

void Events::HandleKeyboardDestroy(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleKeyboardDestroy(listener, data);
}

void Events::HandleKeyboardKey(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleKeyboardKey(listener, data);
}

void Events::HandleKeyboardModifiers(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleKeyboardModifiers(listener, data);
}

void Events::SeatRequestCursor(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->SeatRequestCursor(listener, data);
}

void Events::SeatPointerFocusChange(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->SeatPointerFocusChange(listener, data);
}

void Events::SeatRequestSetSelection(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->SeatRequestSetSelection(listener, data);
}

void Events::HandlePointerDestroy(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandlePointerDestroy(listener, data);
}

void Events::HandleCursorMotion(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleCursorMotion(listener, data);
}

void Events::HandleCursorMotionAbsolute(wl_listener* listener, void* data){
	g_pCompositor->m_InputManager->HandleCursorMotionAbsolute(listener, data);
}

void Events::HandleCursorButton(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleCursorButton(listener, data);
}

void Events::HandleCursorAxis(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleCursorAxis(listener, data);
}

void Events::HandleCursorFrame(wl_listener* listener, void* data) {
	g_pCompositor->m_InputManager->HandleCursorFrame(listener, data);
}