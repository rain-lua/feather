#ifndef SEAT_MANAGER_H
#define SEAT_MANAGER_H

#include "../../../include/Defines.hpp"

class SeatManager {
public:
    SeatManager();

    void Initialize();
    void Cleanup();

    wlr_seat* m_Seat;

    wl_listener m_RequestCursor;
	wl_listener m_PointerFocusChange;
	wl_listener m_RequestSetSelection;

    static void SeatRequestCursor(wl_listener* listener, void* data);
    static void SeatPointerFocusChange(wl_listener* listener, void* data);
    static void SeatRequestSetSelection(wl_listener* listener, void* data);
};

#endif