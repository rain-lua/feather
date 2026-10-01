#pragma once

#include "../../../include/Defines.hpp"

void HandleNewOutput(wl_listener* listener, void* data);
void HandleOutputDestroy(void* owner, void* data);
void HandleOutputRequestState(void* owner, void* data);
void HandleOutputFrame(void* owner, void* data);