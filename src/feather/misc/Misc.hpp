#pragma once

#include "../../include/Defines.hpp"

void AddSignal(wl_signal* signal, wl_listener* listener, void (*callback)(wl_listener*, void*));
void AddNotify(wl_listener* listener, wl_notify_func_t callback);