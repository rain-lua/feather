#pragma once

#include "../../../include/Defines.hpp"

void HandleNewInput(wl_listener* listener, void* data);

void HandleKeyboardDestroy(void* owner, void* data);
void HandleKeyboardKey(void* owner, void* data);
void HandleKeyboardModifiers(void* owner, void* data);

void SeatRequestCursor(wl_listener* listener, void* data);
void SeatPointerFocusChange(wl_listener* listener, void* data);
void SeatRequestSetSelection(wl_listener* listener, void* data);

void HandlePointerDestroy(void* owner, void* data);
void HandleCursorMotion(wl_listener* listener, void* data);
void HandleCursorMotionAbsolute(wl_listener* listener, void* data);
void HandleCursorButton(wl_listener* listener, void* data);
void HandleCursorAxis(wl_listener* listener, void* data);
void HandleCursorFrame(wl_listener* listener, void* data);