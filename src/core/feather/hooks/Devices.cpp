#include "Devices.hpp"

#include "../../feather/Feather.hpp"
#include "../../../debug/Logger.hpp"

void HandleNewInput(wl_listener* listener, void* data) {
    wlr_input_device* device = static_cast<wlr_input_device*>(data);

    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            Logger::Log(
                LogLevel::INFO,
                "--- New Keyboard Connected: %s ---",
                device->name
            );

            g_pFeather->m_InputHandler->HandleNewKeyboard(device);
            break;

        case WLR_INPUT_DEVICE_POINTER:
            Logger::Log(
                LogLevel::INFO,
                "--- New Pointer Connected: %s ---",
                device->name
            );

            g_pFeather->m_InputHandler->HandleNewPointer(device);
            break;

        default:
            Logger::Log(
                LogLevel::INFO,
                "--- New Input Device (%d): %s ---",
                device->type,
                device->name
            );
            break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;

    if (!wl_list_empty(&g_pFeather->m_Keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }

    wlr_seat_set_capabilities(
        g_pFeather->m_Seat,
        caps
    );
}

void HandleKeyboardDestroy(void* owner, void* data) {
    g_pFeather->m_InputHandler->HandleKeyboardDestroy(owner, data);
}

void HandleKeyboardKey(void* owner, void* data) {
    g_pFeather->m_InputHandler->HandleKeyboardKey(owner, data);
}

void HandleKeyboardModifiers(void* owner, void* data) {
    g_pFeather->m_InputHandler->HandleKeyboardModifiers(owner, data);
}

void SeatRequestCursor(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->SeatRequestCursor(listener, data);
}

void SeatPointerFocusChange(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->SeatPointerFocusChange(listener, data);
}

void SeatRequestSetSelection(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->SeatRequestSetSelection(listener, data);
}

void HandlePointerDestroy(void* owner, void* data) {
    g_pFeather->m_InputHandler->HandlePointerDestroy(owner, data);
}

void HandleCursorMotion(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->HandleCursorMotion(listener, data);
}

void HandleCursorMotionAbsolute(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->HandleCursorMotionAbsolute(listener, data);
}

void HandleCursorButton(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->HandleCursorButton(listener, data);
}

void HandleCursorAxis(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->HandleCursorAxis(listener, data);
}

void HandleCursorFrame(wl_listener* listener, void* data) {
    g_pFeather->m_InputHandler->HandleCursorFrame(listener, data);
}