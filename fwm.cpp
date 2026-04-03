#include <iostream>
#include <getopt.h>    
#include <libinput.h>     
#include <linux/input-event-codes.h> 
#include <wayland-server-core.h>

// https://github.com/swaywm/wlroots/issues/682
// stooopid devs make shit not work in c++ so we have to do some class and namespace shit FUCK YOU

#define class WLROOTS
#define namespace KILLYOURSELF
#define static

extern "C" {
    #include <wlr/backend.h>
    #include <wlr/render/allocator.h>
    #include <wlr/render/wlr_renderer.h>
    #include <wlr/types/wlr_cursor.h>
    #include <wlr/types/wlr_compositor.h>
    #include <wlr/types/wlr_data_device.h>
    #include <wlr/types/wlr_input_device.h>
    #include <wlr/types/wlr_keyboard.h>
    #include <wlr/types/wlr_output.h>
    #include <wlr/types/wlr_output_layout.h>
    #include <wlr/types/wlr_pointer.h>
    #include <wlr/types/wlr_scene.h>
    #include <wlr/types/wlr_seat.h>
    #include <wlr/types/wlr_subcompositor.h>
    #include <wlr/types/wlr_xcursor_manager.h>
    #include <wlr/types/wlr_xdg_shell.h>
    #include <wlr/util/log.h>
    #include <xkbcommon/xkbcommon.h>
}

#undef class
#undef namespace
#undef static

//structs

struct fwm_server {
    struct wl_display *wl_display;
    struct wlr_backend *backend;

    struct wlr_seat *seat;
	struct wl_listener new_input;
    struct wl_list keyboards;
};

struct fwm_kb {
	struct wl_list link;
	struct fwm_server *server;
	struct wlr_keyboard *wlr_keyboard;

	struct wl_listener modifiers;
	struct wl_listener key;
	struct wl_listener destroy;
};

//functions

static void fwm_log(const char* fmt, ...);
static void server_new_keyboard(struct fwm_server *server, struct wlr_input_device *device);
static void server_new_input(struct wl_listener *listener, void *data);
static void keyboard_handle_destroy(struct wl_listener *listener, void *data);
static void keyboard_handle_key(struct wl_listener *listener, void *data);
static void keyboard_handle_modifiers(struct wl_listener *listener, void *data);

//main

int main(int argc, char **argv){
    struct fwm_server server;

    server.wl_display = wl_display_create();

    server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
	if (server.backend == NULL) {
		fwm_log("failed to create wlr_backend");
		return 1;
	}

    wl_list_init(&server.keyboards);
	server.new_input.notify = server_new_input;
	wl_signal_add(&server.backend->events.new_input, &server.new_input);
	server.seat = wlr_seat_create(server.wl_display, "seat0");

    const char *socket = wl_display_add_socket_auto(server.wl_display);
	if (!socket) {
		wlr_backend_destroy(server.backend);
		return 1;
	}

	if (!wlr_backend_start(server.backend)) {
		wlr_backend_destroy(server.backend);
		wl_display_destroy(server.wl_display);
		return 1;
	}

    fwm_log("fwm running on WAYLAND_DISPLAY=%s", socket);
    wl_display_run(server.wl_display);

    wl_display_destroy_clients(server.wl_display);

	wlr_backend_destroy(server.backend);
	wl_display_destroy(server.wl_display);

    return 0;

}

static void fwm_log(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    std::cout << "[fwm] " << buffer << std::endl;
}

static void server_new_keyboard(struct fwm_server *server, struct wlr_input_device *device){
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);
    struct fwm_kb *keyboard = (fwm_kb *)calloc(1, sizeof(*keyboard));

	keyboard->server = server;
	keyboard->wlr_keyboard = wlr_keyboard;

	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_keymap_new_from_names(context, NULL,
		XKB_KEYMAP_COMPILE_NO_FLAGS);

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

static void server_new_input(struct wl_listener *listener, void *data){
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

static void keyboard_handle_destroy(struct wl_listener *listener, void *data){
	struct fwm_kb *keyboard =
		wl_container_of(listener, keyboard, destroy);
	wl_list_remove(&keyboard->modifiers.link);
	wl_list_remove(&keyboard->key.link);
	wl_list_remove(&keyboard->destroy.link);
	wl_list_remove(&keyboard->link);
	free(keyboard);
}

static bool handle_keybinding(struct fwm_server *server, xkb_keysym_t sym){
    fwm_log("alt");
    //assumes alt is held down.
	switch (sym) {
	case XKB_KEY_Escape:
        fwm_log("alt+esc");
		wl_display_terminate(server->wl_display);
		break;
	default:
		return false;
	}
	return true;
}

static void keyboard_handle_key(struct wl_listener *listener, void *data){
	struct fwm_kb *keyboard =
		wl_container_of(listener, keyboard, key);
	struct fwm_server *server = keyboard->server;
	struct wlr_keyboard_key_event *event = static_cast<wlr_keyboard_key_event *>(data);
	struct wlr_seat *seat = server->seat;

	uint32_t keycode = event->keycode + 8;

	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->wlr_keyboard->xkb_state, keycode, &syms);

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
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
			event->keycode, event->state);
	}
}

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data){
	struct fwm_kb *keyboard =
		wl_container_of(listener, keyboard, modifiers);

	wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);

	wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
		&keyboard->wlr_keyboard->modifiers);
}