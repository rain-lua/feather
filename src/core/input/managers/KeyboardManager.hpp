#ifndef KEYBOARD_MANAGER_H
#define KEYBOARD_MANAGER_H

#include "../../../include/Defines.hpp"

struct Keyboard {
    wl_list m_Link;
    wlr_keyboard *m_WlrKeyboard;
    
    wl_listener m_Modifiers;
    wl_listener m_Key;
    wl_listener m_Destroy;
};

class KeyboardManager {
public:
    KeyboardManager();

    void Initialize();
    void Cleanup();

    wl_list m_Keyboards;

    std::string m_Layout;

    int m_RepeatRate;
    int m_RepeatDelay;

    static void HandleNewKeyboard(wlr_input_device *device);
    static void HandleKeyboardDestroy(wl_listener *listener, void *data);
    static void HandleKeyboardKey(wl_listener *listener, void *data);
    static void HandleKeyboardModifiers(wl_listener *listener, void *data);
};

#endif