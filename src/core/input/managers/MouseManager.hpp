#ifndef MOUSE_MANAGER_H
#define MOUSE_MANAGER_H

#include "../../../include/Defines.hpp"

class Compositor;

struct Pointer {
    Compositor *m_Server;
    wlr_input_device *m_Device;

    wl_listener m_Destroy;
    wl_list m_Link;
};

enum CursorMode {
	CURSOR_PASSTHROUGH,
	CURSOR_MOVE,
	CURSOR_RESIZE,
};

class MouseManager {
public:
    static void HandleNewPointer(Compositor *server, wlr_input_device *device);
    static void HandlePointerDestroy(wl_listener *listener, void *data);
    static void SeatRequestCursor(wl_listener *listener, void *data);
    static void SeatPointerFocusChange(wl_listener *listener, void *data);
    static void SeatRequestSetSelection(wl_listener *listener, void *data);
    static void ResetCursorMode(Compositor *server);
    static void ProcessCursorMotion(Compositor *server, uint32_t time);
    static void HandleCursorMotion(wl_listener *listener, void *data);
    static void HandleCursorMotionAbsolute(wl_listener *listener, void *data);
    static void HandleCursorButton(wl_listener *listener, void *data);
    static void HandleCursorAxis(wl_listener *listener, void *data);
    static void HandleCursorFrame(wl_listener *listener, void *data);
};

#endif