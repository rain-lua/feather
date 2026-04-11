#ifndef KEYBOARD_MANAGER_H
#define KEYBOARD_MANAGER_H

#include "../../../include/Defines.hpp"

class Compositor;

struct Keyboard {
    wl_list m_Link;
    Compositor *m_Server;
    wlr_keyboard *m_WlrKeyboard;
    wl_listener m_Modifiers;
    wl_listener m_Key;
    wl_listener m_Destroy;
};

class KeyboardManager {
public:
    static void HandleNewKeyboard(Compositor *server, wlr_input_device *device);
    static void HandleKeyboardDestroy(wl_listener *listener, void *data);
    static void HandleKeyboardKey(wl_listener *listener, void *data);
    static void HandleKeyboardModifiers(wl_listener *listener, void *data);
};

#endif