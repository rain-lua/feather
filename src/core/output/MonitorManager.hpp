#ifndef MONITOR_H
#define MONITOR_H

#include "../../include/Defines.hpp"

class Compositor;

struct Monitor {
    wl_list m_Link;
    Compositor *m_Server;
    wlr_output *m_WlrOutput;
    wl_listener m_Frame;
    wl_listener m_RequestState;
    wl_listener m_Destroy;
};

class MonitorManager {
public:
    static void HandleNewOutput(wl_listener *listener, void *data);
    static void HandleOutputDestroy(wl_listener *listener, void *data);
    static void HandleOutputRequestState(wl_listener *listener, void *data);
    static void HandleOutputFrame(wl_listener *listener, void *data);
};

#endif