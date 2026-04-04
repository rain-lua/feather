#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "../../include/Defines.hpp"

class Compositor;

class InputManager {
public:
    static void HandleNewInput(wl_listener *listener, void *data);
};

#endif