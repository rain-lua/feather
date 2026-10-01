#include "Misc.hpp"

void AddSignal(wl_signal* signal, wl_listener* listener, void (*callback)(wl_listener*, void*)) {
    AddNotify(listener, callback);
    wl_signal_add(signal, listener);
}

void AddNotify(wl_listener* listener, wl_notify_func_t callback) {
    listener->notify = callback;
}