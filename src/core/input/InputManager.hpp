#pragma once

#include "../../include/Defines.hpp"

class InputManager {
public:
    InputManager();
    ~InputManager();

    wl_listener m_NewInput;

    static void HandleNewInput(wl_listener* listener, void* data);
};