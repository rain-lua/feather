#include "InputManager.hpp"
#include "managers/KeyboardManager.hpp"
#include "managers/MouseManager.hpp"
#include "../compositor/Compositor.hpp"
#include "../../debug/Debug.hpp"

InputManager::InputManager() {
    // todo: add stuff here
}

void InputManager::Initialize() {
    m_NewInput.notify = InputManager::HandleNewInput;
    wl_signal_add(&g_pCompositor->m_Backend->events.new_input, &m_NewInput);
}

void InputManager::Cleanup() {
    wl_list_remove(&m_NewInput.link);
}

void InputManager::HandleNewInput(wl_listener* listener, void* data) {
    wlr_input_device* device = static_cast<wlr_input_device *>(data);

    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD:
            log_info("--- New Keyboard Connected: %s ---", device->name);
            KeyboardManager::HandleNewKeyboard(device);
            break;
        case WLR_INPUT_DEVICE_POINTER:
            log_info("--- New Pointer Connected: %s ---", device->name);
            MouseManager::HandleNewPointer(device);
            break;
        default:
            log_info("--- New Input Device (%d): %s ---", device->type, device->name);
            break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&g_pCompositor->m_KeyboardManager.m_Keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    
    wlr_seat_set_capabilities(g_pCompositor->m_Seat, caps);
}