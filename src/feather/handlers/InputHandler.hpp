#pragma once

#include "../../include/Defines.hpp"

class InputHandler {
  public:
    InputHandler()  = default;
    ~InputHandler() = default;

    static void SeatRequestCursor(wl_listener* listener, void* data);
    static void SeatPointerFocusChange(wl_listener* listener, void* data);
    static void SeatRequestSetSelection(wl_listener* listener, void* data);

    static void HandleNewKeyboard(wlr_input_device* device);
    static void HandleKeyboardDestroy(void* owner, void* data);
    static void HandleKeyboardKey(void* owner, void* data);
    static void HandleKeyboardModifiers(void* owner, void* data);

    static void HandleNewPointer(wlr_input_device* device);
    static void HandlePointerDestroy(void* owner, void* data);
    static void HandleCursorMotion(wl_listener* listener, void* data);
    static void HandleCursorMotionAbsolute(wl_listener* listener, void* data);
    static void HandleCursorButton(wl_listener* listener, void* data);
    static void HandleCursorAxis(wl_listener* listener, void* data);
    static void HandleCursorFrame(wl_listener* listener, void* data);

    void        ProcessCursorMotion(uint32_t time);
    void        ResetCursorMode();
};