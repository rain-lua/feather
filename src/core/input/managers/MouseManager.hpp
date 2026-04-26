#ifndef MOUSE_MANAGER_H
#define MOUSE_MANAGER_H

#include "../../../include/Defines.hpp"

struct Pointer {
    wlr_input_device* m_Device;

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
    MouseManager();

    void Initialize();
    void Cleanup();

    wl_list m_Pointers;

    wlr_xcursor_manager* m_XCursorManager;
	wlr_cursor* m_Cursor;
	
	wl_listener m_CursorMotion;
	wl_listener m_CursorMotionAbsolute;
	wl_listener m_CursorButton;
	wl_listener m_CursorAxis;
	wl_listener m_CursorFrame;

    wl_listener m_RequestCursor;
	wl_listener m_PointerFocusChange;
	wl_listener m_RequestSetSelection;
    
    CursorMode m_CursorMode;

    static void HandleNewPointer(wlr_input_device* device);
    static void HandlePointerDestroy(wl_listener* listener, void* data);
    static void SeatRequestCursor(wl_listener* listener, void* data);
    static void SeatPointerFocusChange(wl_listener* listener, void* data);
    static void SeatRequestSetSelection(wl_listener* listener, void* data);
    static void ResetCursorMode();
    static void ProcessCursorMotion(uint32_t time);
    static void HandleCursorMotion(wl_listener* listener, void* data);
    static void HandleCursorMotionAbsolute(wl_listener* listener, void* data);
    static void HandleCursorButton(wl_listener* listener, void* data);
    static void HandleCursorAxis(wl_listener* listener, void* data);
    static void HandleCursorFrame(wl_listener* listener, void* data);
};

#endif