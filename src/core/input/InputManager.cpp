#include "InputManager.hpp"
#include "managers/KeyboardManager.hpp"
#include "managers/MouseManager.hpp"
#include "../compositor/Compositor.hpp"
#include "../../debug/Debug.hpp"

void InputManager::HandleNewInput(wl_listener *listener, void *data) {
    Compositor *server = wl_container_of(listener, server, m_NewInput);
    wlr_input_device *device = static_cast<wlr_input_device *>(data);

    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            log_info("--- New Keyboard Connected: %s ---", device->name);
            KeyboardManager::HandleNewKeyboard(server, device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            log_info("--- New Pointer Connected: %s ---", device->name);
            MouseManager::HandleNewPointer(server, device);
            break;
        default:
            log_info("--- New Input Device (%d): %s ---", device->type, device->name);
            break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->m_Keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    
    wlr_seat_set_capabilities(server->m_Seat, caps);
}