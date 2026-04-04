#ifndef STRUCTS_H
#define STRUCTS_H

#include "../../include/defines.hpp"

struct fwm_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;

    struct wlr_seat *seat;
    struct wl_listener new_input;
    struct wl_list keyboards;
};

struct fwm_output {
    struct wl_list link;
    struct fwm_server *server;
    struct wlr_output *wlr_output;
    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

struct fwm_kb {
    struct wl_list link;
    struct fwm_server *server;
    struct wlr_keyboard *wlr_keyboard;

    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

#endif