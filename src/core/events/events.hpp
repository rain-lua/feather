#ifndef EVENTS_H
#define EVENTS_H

#include "../../include/defines.hpp"
#include "../../debug/debug.hpp"
#include "../types/structs.hpp"
#include "events.hpp"

void server_new_keyboard(struct fwm_server *server, struct wlr_input_device *device);
void server_new_input(struct wl_listener *listener, void *data);
void keyboard_handle_destroy(struct wl_listener *listener, void *data);
void keyboard_handle_key(struct wl_listener *listener, void *data);
void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
void server_new_output(struct wl_listener *listener, void *data);
void output_destroy(struct wl_listener *listener, void *data);
void output_request_state(struct wl_listener *listener, void *data);
void output_frame(struct wl_listener *listener, void *data);

#endif