#pragma once

#include "../../include/Defines.hpp"

void HandleNewWindow(wl_listener* listener, void* data);
void HandleWindowMap(void* owner, void* data);
void HandleWindowUnmap(void* owner, void* data);
void HandleWindowCommit(void* owner, void* data);
void HandleWindowDestroy(void* owner, void* data);
void HandleWindowRequestMove(void* owner, void* data);
void HandleWindowRequestResize(void* owner, void* data);
void HandleWindowRequestMaximize(void* owner, void* data);
void HandleWindowRequestFullscreen(void* owner, void* data);