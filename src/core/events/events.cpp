#include "events.hpp"

void server_new_input(struct wl_listener *listener, void *data){
    struct fwm_server *server =
        wl_container_of(listener, server, new_input);

    struct wlr_input_device *device = static_cast<wlr_input_device *>(data);
    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        server_new_keyboard(server, device);
        break;
    default:
        break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

void server_new_keyboard(struct fwm_server *server, struct wlr_input_device *device){
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
    struct fwm_kb *keyboard = (fwm_kb *)calloc(1, sizeof(*keyboard));

    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);

    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);

    keyboard->destroy.notify = keyboard_handle_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

    wl_list_insert(&server->keyboards, &keyboard->link);
}

void keyboard_handle_destroy(struct wl_listener *listener, void *data){
    struct fwm_kb *keyboard =
        wl_container_of(listener, keyboard, destroy);
    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);
    free(keyboard);
}

bool handle_keybinding(struct fwm_server *server, xkb_keysym_t sym){
    log_debug("alt");
    //assumes alt is held down.
    switch (sym) {
    case XKB_KEY_Escape:
        log_debug("alt+esc");
        wl_display_terminate(server->wl_display);
        break;
    default:
        return false;
    }
    return true;
}

void keyboard_handle_key(struct wl_listener *listener, void *data){
    struct fwm_kb *keyboard = wl_container_of(listener, keyboard, key);
    struct fwm_server *server = keyboard->server;
    struct wlr_keyboard_key_event *event = static_cast<wlr_keyboard_key_event *>(data);
    struct wlr_seat *seat = server->seat;

    uint32_t keycode = event->keycode + 8;

    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
    if ((modifiers & WLR_MODIFIER_ALT) &&
            event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {

        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybinding(server, syms[i]);
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode, event->state);
    }
}

void keyboard_handle_modifiers(struct wl_listener *listener, void *data){
    struct fwm_kb *keyboard = wl_container_of(listener, keyboard, modifiers);

    wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);

    wlr_seat_keyboard_notify_modifiers(keyboard->server->seat, &keyboard->wlr_keyboard->modifiers);
}

void server_new_output(struct wl_listener *listener, void *data){
    struct fwm_server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = static_cast<struct wlr_output *>(data);

    log_info("new output");

    wlr_output_init_render(wlr_output, server->allocator, server->renderer);

    struct wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);

    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    if (mode != NULL) {
        wlr_output_state_set_mode(&state, mode);
    }

    wlr_output_commit_state(wlr_output, &state);
    wlr_output_state_finish(&state);

    struct fwm_output *output = (fwm_output *)calloc(1, sizeof(*output));
    output->wlr_output = wlr_output;
    output->server = server;

    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->request_state.notify = output_request_state;
    wl_signal_add(&wlr_output->events.request_state, &output->request_state);

    output->destroy.notify = output_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    wl_list_insert(&server->outputs, &output->link);

    struct wlr_output_layout_output *l_output = wlr_output_layout_add_auto(server->output_layout, wlr_output);
    struct wlr_scene_output *scene_output = wlr_scene_output_create(server->scene, wlr_output);
    wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
}

void output_destroy(struct wl_listener *listener, void *data){
    struct fwm_output *output = wl_container_of(listener, output, destroy);

    log_info("output destroy");

    wl_list_remove(&output->frame.link);
    wl_list_remove(&output->request_state.link);
    wl_list_remove(&output->destroy.link);
    wl_list_remove(&output->link);
    free(output);
}

void output_request_state(struct wl_listener *listener, void *data){
    log_debug("output request");

    struct fwm_output *output = wl_container_of(listener, output, request_state);
    const struct wlr_output_event_request_state *event = static_cast<wlr_output_event_request_state *>(data);
    wlr_output_commit_state(output->wlr_output, event->state);
}

void output_frame(struct wl_listener *listener, void *data){
    log_debug("output frame");

    struct fwm_output *output = wl_container_of(listener, output, frame);
    struct wlr_scene *scene = output->server->scene;

    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output->wlr_output);

    wlr_scene_output_commit(scene_output, NULL);

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}