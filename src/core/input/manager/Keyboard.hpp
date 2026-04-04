#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../../../include/Defines.hpp"

class Compositor;

class KeyboardManager {
public:
    static void HandleNewKeyboard(Compositor *server, wlr_input_device *device);
    static void HandleKeyboardDestroy(wl_listener *listener, void *data);
    static void HandleKeyboardKey(wl_listener *listener, void *data);
    static void HandleKeyboardModifiers(wl_listener *listener, void *data);
};

class Keyboard {
public:
    wl_list m_Link;
    Compositor *m_Server;
    wlr_keyboard *m_WlrKeyboard;
    wl_listener m_Modifiers;
    wl_listener m_Key;
    wl_listener m_Destroy;
};

#endif