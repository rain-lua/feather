#include "KeyboardManager.hpp"
#include "../../compositor/Compositor.hpp"
#include "../../../debug/Debug.hpp"
#include "../../util/Util.hpp"

KeyboardManager::KeyboardManager() {
    wl_list_init(&m_Keyboards);
}

void KeyboardManager::Initialize() {
    m_Layout = g_pCompositor->m_ConfigManager->GetRootTree()->GetLeaf("layout")->GetString();

    m_RepeatRate = g_pCompositor->m_ConfigManager->GetRootTree()->GetLeaf("repeat_rate")->GetInt();
    m_RepeatDelay = g_pCompositor->m_ConfigManager->GetRootTree()->GetLeaf("repeat_delay")->GetInt();
}

void KeyboardManager::Cleanup() {
    // we don't have to do anything here yet
}

void KeyboardManager::HandleNewKeyboard(wlr_input_device *device) {
    wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    Keyboard *keyboard = (Keyboard *)calloc(1, sizeof(*keyboard));
    keyboard->m_WlrKeyboard = wlr_keyboard;

    xkb_rule_names names;
    memset(&names, 0, sizeof(names));
    names.layout = g_pCompositor->m_KeyboardManager.m_Layout.c_str(); 

    xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    xkb_keymap *keymap = xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(wlr_keyboard, g_pCompositor->m_KeyboardManager.m_RepeatRate, g_pCompositor->m_KeyboardManager.m_RepeatDelay);

    keyboard->m_Modifiers.notify = KeyboardManager::HandleKeyboardModifiers;
    keyboard->m_Key.notify = KeyboardManager::HandleKeyboardKey;
    keyboard->m_Destroy.notify = KeyboardManager::HandleKeyboardDestroy;

    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->m_Modifiers);
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->m_Key);
    wl_signal_add(&device->events.destroy, &keyboard->m_Destroy);

    wlr_seat_set_keyboard(g_pCompositor->m_Seat, keyboard->m_WlrKeyboard);
    wl_list_insert(&g_pCompositor->m_KeyboardManager.m_Keyboards, &keyboard->m_Link);
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

static bool HandleKeybinding(xkb_keysym_t sym, uint32_t mods) {
    const bool super = mods & WLR_MODIFIER_LOGO;

    if (super) {
        switch (sym) {
            case XKB_KEY_q:
                Spawn("kitty");
                return true;
            case XKB_KEY_c:
                g_pCompositor->m_WindowManager.CloseWindow(g_pCompositor->m_WindowManager.m_FocusedWindow);
                return true;
            case XKB_KEY_Escape:
                wl_display_terminate(g_pCompositor->m_Display);
                return true;
            default:
                break;
        }
    }

    return false;
}

void KeyboardManager::HandleKeyboardKey(wl_listener *listener, void *data) {
    Keyboard *keyboard = wl_container_of(listener, keyboard, m_Key);

    wlr_keyboard_key_event *event = static_cast<wlr_keyboard_key_event *>(data);
    wlr_seat *seat = g_pCompositor->m_Seat;

    uint32_t keycode = ToXKBKeycode(event->keycode);
    const xkb_keysym_t *syms;

    int nsyms = xkb_state_key_get_syms(keyboard->m_WlrKeyboard->xkb_state, keycode, &syms);
    uint32_t mods = wlr_keyboard_get_modifiers(keyboard->m_WlrKeyboard);

    bool handled = false;

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            if (!handled) {
                handled = HandleKeybinding(syms[i], mods);
            }

            if (handled) {
                break;
            }
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(seat, keyboard->m_WlrKeyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
    }
}

void KeyboardManager::HandleKeyboardModifiers(wl_listener *listener, void *data) {
    Keyboard *keyboard = wl_container_of(listener, keyboard, m_Modifiers);

    wlr_seat_set_keyboard(g_pCompositor->m_Seat, keyboard->m_WlrKeyboard);
    wlr_seat_keyboard_notify_modifiers(g_pCompositor->m_Seat, &keyboard->m_WlrKeyboard->modifiers);
}