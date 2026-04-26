#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "../../include/Defines.hpp"

class InputManager {
public:
    InputManager();

    void Initialize();
    void Cleanup();

    wl_listener m_NewInput;

    static void HandleNewInput(wl_listener* listener, void* data);
};

#endif