#pragma once

#include "../../../include/Defines.hpp"

class InputManager {
public:
    InputManager() = default;
    ~InputManager() = default;

    static void SeatRequestCursor(wl_listener* listener, void* data);
    static void SeatPointerFocusChange(wl_listener* listener, void* data);
    static void SeatRequestSetSelection(wl_listener* listener, void* data);

    static void HandleNewKeyboard(wlr_input_device* device);
    static void HandleKeyboardDestroy(wl_listener* listener, void* data);
    static void HandleKeyboardKey(wl_listener* listener, void* data);
    static void HandleKeyboardModifiers(wl_listener* listener, void* data);

    static void HandleNewPointer(wlr_input_device* device);
    static void HandlePointerDestroy(wl_listener* listener, void* data);
    static void HandleCursorMotion(wl_listener* listener, void* data);
    static void HandleCursorMotionAbsolute(wl_listener* listener, void* data);
    static void HandleCursorButton(wl_listener* listener, void* data);
    static void HandleCursorAxis(wl_listener* listener, void* data);
    static void HandleCursorFrame(wl_listener* listener, void* data);

    void ProcessCursorMotion(uint32_t time);
    void ResetCursorMode();
};