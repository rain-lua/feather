#include "KeyboardManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

void KeyboardManager::HandleNewKeyboard(Compositor *server, wlr_input_device *device) {
    wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
    Keyboard *keyboard = (Keyboard *)calloc(1, sizeof(*keyboard));
    keyboard->m_Server = server;
    keyboard->m_WlrKeyboard = wlr_keyboard;

    std::string layout = server->m_ConfigManager->GetRootTree()->GetLeaf("layout")->GetString();
    int repeat_rate = server->m_ConfigManager->GetRootTree()->GetLeaf("repeat_rate")->GetInt();
    int repeat_delay = server->m_ConfigManager->GetRootTree()->GetLeaf("repeat_delay")->GetInt();

    xkb_rule_names names;
    memset(&names, 0, sizeof(names));
    names.layout = layout.c_str(); 

    xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    xkb_keymap *keymap = xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(wlr_keyboard, repeat_rate, repeat_delay);

    keyboard->m_Modifiers.notify = KeyboardManager::HandleKeyboardModifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->m_Modifiers);

    keyboard->m_Key.notify = KeyboardManager::HandleKeyboardKey;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->m_Key);

    keyboard->m_Destroy.notify = KeyboardManager::HandleKeyboardDestroy;
    wl_signal_add(&device->events.destroy, &keyboard->m_Destroy);

    wlr_seat_set_keyboard(server->m_Seat, keyboard->m_WlrKeyboard);
    wl_list_insert(&server->m_Keyboards, &keyboard->m_Link);
}

void KeyboardManager::HandleKeyboardDestroy(wl_listener *listener, void *data) {
    log_info("--- Keyboard Disconnected ---");
    Keyboard *keyboard = wl_container_of(listener, keyboard, m_Destroy);
    wl_list_remove(&keyboard->m_Modifiers.link);
    wl_list_remove(&keyboard->m_Key.link);
    wl_list_remove(&keyboard->m_Destroy.link);
    wl_list_remove(&keyboard->m_Link);
    free(keyboard);
}

static bool HandleKeybinding(Compositor *server, xkb_keysym_t sym){
    log_debug("super");

    switch (sym) {
    case XKB_KEY_Escape:
        log_debug("super+esc");
        wl_display_terminate(server->m_Display);
        break;
    case XKB_KEY_q:
        log_debug("super+Q");
        spawn("kitty");
        break;
    case XKB_KEY_c:
        log_debug("super+C");
        kill(server->m_FocusedWindow);
        break;
    default:
        return false;
    }
    
    return true;
}

void KeyboardManager::HandleKeyboardKey(wl_listener *listener, void *data) {
    Keyboard *keyboard = wl_container_of(listener, keyboard, m_Key);
    Compositor *server = keyboard->m_Server;
    wlr_keyboard_key_event *event = static_cast<wlr_keyboard_key_event *>(data);
    wlr_seat *seat = server->m_Seat;

    uint32_t keycode = toXKBKeycode(event->keycode);
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(keyboard->m_WlrKeyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->m_WlrKeyboard);
    if ((modifiers & WLR_MODIFIER_LOGO) && event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            handled = HandleKeybinding(server, syms[i]);
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(seat, keyboard->m_WlrKeyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
    }
}

void KeyboardManager::HandleKeyboardModifiers(wl_listener *listener, void *data) {
    Keyboard *keyboard = wl_container_of(listener, keyboard, m_Modifiers);
    wlr_seat_set_keyboard(keyboard->m_Server->m_Seat, keyboard->m_WlrKeyboard);
    wlr_seat_keyboard_notify_modifiers(keyboard->m_Server->m_Seat, &keyboard->m_WlrKeyboard->modifiers);
}