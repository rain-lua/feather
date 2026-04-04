#include "InputManager.hpp"
#include "../compositor/Compositor.hpp"
#include "manager/Keyboard.hpp"

void InputManager::HandleNewInput(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_NewInput);
    wlr_input_device *device = static_cast<wlr_input_device *>(data);

    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            KeyboardManager::HandleNewKeyboard(server, device);
            break;
        default:
            break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->m_Keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->m_Seat, caps);
}