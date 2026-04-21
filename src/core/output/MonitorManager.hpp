#ifndef MONITOR_MANAGER_H
#define MONITOR_MANAGER_H

#include "../../include/Defines.hpp"

class Compositor;

struct Monitor {
    wl_list m_Link;

    wlr_output *m_WlrOutput;
    wl_listener m_Frame;
    wl_listener m_RequestState;
    wl_listener m_Destroy;
};

class MonitorManager {
public:
    MonitorManager();

    void Initialize();
    void Cleanup();

    wl_list m_Outputs;
    wl_listener m_NewOutput;

    static void HandleNewOutput(wl_listener *listener, void *data);
    static void HandleOutputDestroy(wl_listener *listener, void *data);
    static void HandleOutputRequestState(wl_listener *listener, void *data);
    static void HandleOutputFrame(wl_listener *listener, void *data);
};

#endif