#include "InputHandler.hpp"

#include "../../feather/Feather.hpp"
#include "../../util/Util.hpp"
#include "../../../debug/Logger.hpp"

void InputHandler::SeatRequestCursor(wl_listener* listener, void* data) {
    wlr_seat_pointer_request_set_cursor_event* event = static_cast<wlr_seat_pointer_request_set_cursor_event*>(data);

    wlr_seat_client* focused_client = g_pFeather->m_Seat->pointer_state.focused_client;

    if (focused_client == event->seat_client) {
        wlr_cursor_set_surface(
            g_pFeather->m_Cursor,
            event->surface,
            event->hotspot_x,
            event->hotspot_y
        );
    }
}

void InputHandler::SeatPointerFocusChange(wl_listener* listener, void* data) {
    wlr_seat_pointer_focus_change_event* event = static_cast<wlr_seat_pointer_focus_change_event*>(data);

    if (event->new_surface == nullptr) {
        wlr_cursor_set_xcursor(
            g_pFeather->m_Cursor,
            g_pFeather->m_XCursorManager,
            "default"
        );
    }
}

void InputHandler::SeatRequestSetSelection(wl_listener* listener, void* data) {
    wlr_seat_request_set_selection_event* event = static_cast<wlr_seat_request_set_selection_event*>(data);

    wlr_seat_set_selection(
        g_pFeather->m_Seat,
        event->source,
        event->serial
    );
}

void InputHandler::HandleNewKeyboard(wlr_input_device* device) {
    wlr_keyboard* wlr_keyboard = wlr_keyboard_from_input_device(device);

    Keyboard* keyboard = new Keyboard;

    keyboard->m_WlrKeyboard = wlr_keyboard;

    xkb_rule_names names;
    memset(&names, 0, sizeof(names));

    std::string layout = g_pFeather->m_ConfigManager->GetString("input.keyboard.layout");

    names.layout = layout.c_str();

    xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    xkb_keymap* keymap = xkb_keymap_new_from_names(context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(
        wlr_keyboard,
        keymap
    );

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(wlr_keyboard, g_pFeather->m_ConfigManager->GetInt("input.keyboard.repeat_rate"),
        g_pFeather->m_ConfigManager->GetInt("input.keyboard.repeat_delay")
    );

    keyboard->m_Modifiers.Init(
        &wlr_keyboard->events.modifiers,
        keyboard,
        HandleKeyboardModifiers
    );

    keyboard->m_Key.Init(
        &wlr_keyboard->events.key,
        keyboard,
        HandleKeyboardKey
    );

    keyboard->m_Destroy.Init(
        &device->events.destroy,
        keyboard,
        HandleKeyboardDestroy
    );

    wlr_seat_set_keyboard(
        g_pFeather->m_Seat,
        keyboard->m_WlrKeyboard
    );

    wl_list_insert(
        &g_pFeather->m_Keyboards,
        &keyboard->m_Link
    );
}

void InputHandler::HandleKeyboardDestroy(void* owner, void* data) {
    Keyboard* keyboard = static_cast<Keyboard*>(owner);

    if (!keyboard) {
        return;
    }

    Logger::Log(
        LogLevel::INFO,
        "--- Keyboard Disconnected ---"
    );

    keyboard->m_Modifiers.Remove();
    keyboard->m_Key.Remove();
    keyboard->m_Destroy.Remove();

    wl_list_remove(&keyboard->m_Link);

    delete keyboard;
}

static bool HandleKeybinding(xkb_keysym_t sym,uint32_t mods) {
    const bool super = mods & WLR_MODIFIER_LOGO;

    if (super) {
        switch (sym) {
            case XKB_KEY_q:
                g_pFeather->m_ConfigManager->ExecProc("kitty");
                return true;

            case XKB_KEY_c:
                g_pFeather->CloseWindow(g_pFeather->m_FocusedWindow);
                return true;

            case XKB_KEY_Escape:
                g_pFeather->Stop();
                return true;

            default:
                break;
        }
    }

    return false;
}

void InputHandler::HandleKeyboardKey(void* owner, void* data) {
    Keyboard* keyboard = static_cast<Keyboard*>(owner);

    wlr_keyboard_key_event* event = static_cast<wlr_keyboard_key_event*>(data);

    wlr_seat* seat = g_pFeather->m_Seat;

    uint32_t keycode = ToXKBKeycode(event->keycode);

    const xkb_keysym_t* syms;

    int nsyms =
        xkb_state_key_get_syms(
            keyboard->m_WlrKeyboard->xkb_state,
            keycode,
            &syms
        );

    uint32_t mods =
        wlr_keyboard_get_modifiers(
            keyboard->m_WlrKeyboard
        );

    bool handled = false;

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            if (!handled) {
                handled = HandleKeybinding(
                    syms[i],
                    mods
                );
            }

            if (handled) {
                break;
            }
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(
            seat,
            keyboard->m_WlrKeyboard
        );

        wlr_seat_keyboard_notify_key(
            seat,
            event->time_msec,
            event->keycode,
            event->state
        );
    }
}

void InputHandler::HandleKeyboardModifiers(void* owner, void* data) {
    Keyboard* keyboard = static_cast<Keyboard*>(owner);

    wlr_seat_set_keyboard(
        g_pFeather->m_Seat,
        keyboard->m_WlrKeyboard
    );

    wlr_seat_keyboard_notify_modifiers(
        g_pFeather->m_Seat,
        &keyboard->m_WlrKeyboard->modifiers
    );
}

void InputHandler::HandleNewPointer(wlr_input_device* device) {
    Pointer* pointer = new Pointer;

    pointer->m_Device = device;

    pointer->m_Destroy.Init(
        &device->events.destroy,
        pointer,
        HandlePointerDestroy
    );

    wl_list_insert(
        &g_pFeather->m_Pointers,
        &pointer->m_Link
    );

    wlr_cursor_attach_input_device(
        g_pFeather->m_Cursor,
        device
    );
}

void InputHandler::HandlePointerDestroy(void* owner, void* data) {
    Pointer* pointer = static_cast<Pointer*>(owner);

    if (!pointer) {
        return;
    }

    Logger::Log(
        LogLevel::INFO,
        "--- Pointer Disconnected ---"
    );

    pointer->m_Destroy.Remove();

    wl_list_remove(&pointer->m_Link);

    delete pointer;
}

void InputHandler::HandleCursorMotion(wl_listener* listener, void* data) {
    wlr_pointer_motion_event* event = static_cast<wlr_pointer_motion_event*>(data);

    wlr_cursor_move(
        g_pFeather->m_Cursor,
        &event->pointer->base,
        event->delta_x,
        event->delta_y
    );

    g_pFeather->m_InputHandler->ProcessCursorMotion(
        event->time_msec
    );
}

void InputHandler::HandleCursorMotionAbsolute(wl_listener* listener, void* data) {
    wlr_pointer_motion_absolute_event* event = static_cast<wlr_pointer_motion_absolute_event*>(data);

    wlr_cursor_warp_absolute(
        g_pFeather->m_Cursor,
        &event->pointer->base,
        event->x,
        event->y
    );

    g_pFeather->m_InputHandler->ProcessCursorMotion(
        event->time_msec
    );
}

void InputHandler::HandleCursorButton(wl_listener* listener, void* data) {
    wlr_pointer_button_event* event = static_cast<wlr_pointer_button_event*>(data);

    wlr_seat_pointer_notify_button(
        g_pFeather->m_Seat,
        event->time_msec,
        event->button,
        event->state
    );

    if (event->state == WL_POINTER_BUTTON_STATE_RELEASED) {
        g_pFeather->m_InputHandler->ResetCursorMode();
    } else {
        double sx;
        double sy;

        wlr_surface* surface = nullptr;

        Window* window =
            g_pFeather->FindWindowAt(
                g_pFeather->m_Cursor->x,
                g_pFeather->m_Cursor->y,
                &surface,
                &sx,
                &sy
            );

        g_pFeather->FocusWindow(window);
    }
}

void InputHandler::HandleCursorAxis(wl_listener* listener, void* data) {
    wlr_pointer_axis_event* event = static_cast<wlr_pointer_axis_event*>(data);

    wlr_seat_pointer_notify_axis(
        g_pFeather->m_Seat,
        event->time_msec,
        event->orientation,
        event->delta,
        event->delta_discrete,
        event->source,
        event->relative_direction
    );
}

void InputHandler::HandleCursorFrame(wl_listener* listener, void* data) {
    wlr_seat_pointer_notify_frame(
        g_pFeather->m_Seat
    );
}

void InputHandler::ResetCursorMode() {
    g_pFeather->m_CursorMode = CURSOR_PASSTHROUGH;
}

void InputHandler::ProcessCursorMotion(uint32_t time) {
    if (g_pFeather->m_CursorMode == CURSOR_MOVE) {
        return;
    }

    if (g_pFeather->m_CursorMode == CURSOR_RESIZE) {
        return;
    }

    double sx;
    double sy;

    wlr_seat* seat = g_pFeather->m_Seat;
    wlr_surface* surface = nullptr;

    wlr_scene_node* node =
        wlr_scene_node_at(
            &g_pFeather->m_Scene->tree.node,
            g_pFeather->m_Cursor->x,
            g_pFeather->m_Cursor->y,
            &sx,
            &sy
        );

    if (!node) {
        wlr_cursor_set_xcursor(
            g_pFeather->m_Cursor,
            g_pFeather->m_XCursorManager,
            "default"
        );
    }

    if (node && node->type == WLR_SCENE_NODE_BUFFER) {
        wlr_scene_buffer* scene_buffer =
            wlr_scene_buffer_from_node(node);

        wlr_scene_surface* scene_surface =
            wlr_scene_surface_try_from_buffer(scene_buffer);

        if (scene_surface) {
            surface = scene_surface->surface;
        }
    }

    if (surface) {
        wlr_seat_pointer_notify_enter(
            seat,
            surface,
            sx,
            sy
        );

        wlr_seat_pointer_notify_motion(
            seat,
            time,
            sx,
            sy
        );
    } else {
        wlr_seat_pointer_clear_focus(seat);
    }
}