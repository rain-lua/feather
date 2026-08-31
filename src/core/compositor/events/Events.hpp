#pragma once

#include "../../compositor/Compositor.hpp"

namespace Events {
    // MONITORS

    void HandleNewOutput(wl_listener* listener, void* data);
    void HandleOutputDestroy(wl_listener* listener, void* data);
    void HandleOutputRequestState(wl_listener* listener, void* data);
    void HandleOutputFrame(wl_listener* listener, void* data);

    // INPUT

    void HandleNewInput(wl_listener* listener, void* data);

    void HandleKeyboardDestroy(wl_listener* listener, void* data);
    void HandleKeyboardKey(wl_listener* listener, void* data);
    void HandleKeyboardModifiers(wl_listener* listener, void* data);

    void SeatRequestCursor(wl_listener* listener, void* data);
    void SeatPointerFocusChange(wl_listener* listener, void* data);
    void SeatRequestSetSelection(wl_listener* listener, void* data);

    void HandlePointerDestroy(wl_listener* listener, void* data);
    void HandleCursorMotion(wl_listener* listener, void* data);
    void HandleCursorMotionAbsolute(wl_listener* listener, void* data);
    void HandleCursorButton(wl_listener* listener, void* data);
    void HandleCursorAxis(wl_listener* listener, void* data);
    void HandleCursorFrame(wl_listener* listener, void* data);

    // WINDOWS

    void HandleNewWindow(wl_listener* listener, void* data);
    void HandleWindowMap(wl_listener* listener, void* data);
    void HandleWindowUnmap(wl_listener* listener, void* data);
    void HandleWindowCommit(wl_listener* listener, void* data);
    void HandleWindowDestroy(wl_listener* listener, void* data);
    void HandleWindowRequestMove(wl_listener* listener, void* data);
    void HandleWindowRequestResize(wl_listener* listener, void* data);
    void HandleWindowRequestMaximize(wl_listener* listener, void* data);
    void HandleWindowRequestFullscreen(wl_listener* listener, void* data);
}