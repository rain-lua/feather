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

//functions

static void fwm_log(const char* fmt, ...);
static void server_new_keyboard(struct fwm_server *server, struct wlr_input_device *device);
static void server_new_input(struct wl_listener *listener, void *data);
static void keyboard_handle_destroy(struct wl_listener *listener, void *data);
static void keyboard_handle_key(struct wl_listener *listener, void *data);
static void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
static void server_new_output(struct wl_listener *listener, void *data);
static void output_destroy(struct wl_listener *listener, void *data);
static void output_request_state(struct wl_listener *listener, void *data);
static void output_frame(struct wl_listener *listener, void *data);

//main

int main(int argc, char **argv){
    struct fwm_server server = {0};

    server.wl_display = wl_display_create();

    server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
	if (server.backend == NULL) {
		fwm_log("failed to create wlr_backend");
		return 1;
	}

	server.renderer = wlr_renderer_autocreate(server.backend);
	if (server.renderer == NULL) {
		fwm_log("failed to create wlr_renderer");
		return 1;
	}

	wlr_renderer_init_wl_display(server.renderer, server.wl_display);

	server.allocator = wlr_allocator_autocreate(server.backend,
		server.renderer);
	if (server.allocator == NULL) {
		fwm_log("failed to create wlr_allocator");
		return 1;
	}

	server.output_layout = wlr_output_layout_create(server.wl_display);
	wl_list_init(&server.outputs);
	server.new_output.notify = server_new_output;
	wl_signal_add(&server.backend->events.new_output, &server.new_output);

	server.scene = wlr_scene_create();
	server.scene_layout = wlr_scene_attach_output_layout(server.scene, server.output_layout);

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

	wl_list_remove(&server.new_input.link);
	wl_list_remove(&server.new_output.link);

	wlr_backend_destroy(server.backend);
	wlr_allocator_destroy(server.allocator);
	wlr_renderer_destroy(server.renderer);
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


static void server_new_output(struct wl_listener *listener, void *data){
	struct fwm_server *server =
		wl_container_of(listener, server, new_output);
	struct wlr_output *wlr_output = static_cast<struct wlr_output *>(data);

	fwm_log("new output");

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

	struct wlr_output_layout_output *l_output = wlr_output_layout_add_auto(server->output_layout,
		wlr_output);
	struct wlr_scene_output *scene_output = wlr_scene_output_create(server->scene, wlr_output);
	wlr_scene_output_layout_add_output(server->scene_layout, l_output, scene_output);
}

static void output_destroy(struct wl_listener *listener, void *data){
	struct fwm_output *output = wl_container_of(listener, output, destroy);

	fwm_log("output destroy");

	wl_list_remove(&output->frame.link);
	wl_list_remove(&output->request_state.link);
	wl_list_remove(&output->destroy.link);
	wl_list_remove(&output->link);
	free(output);
}

static void output_request_state(struct wl_listener *listener, void *data){
	fwm_log("output request");

	struct fwm_output *output = wl_container_of(listener, output, request_state);
	const struct wlr_output_event_request_state *event = static_cast<wlr_output_event_request_state *>(data);
	wlr_output_commit_state(output->wlr_output, event->state);
}


static void output_frame(struct wl_listener *listener, void *data){
	fwm_log("output frame");

	struct fwm_output *output = wl_container_of(listener, output, frame);
	struct wlr_scene *scene = output->server->scene;

	struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(
		scene, output->wlr_output);

	wlr_scene_output_commit(scene_output, NULL);

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	wlr_scene_output_send_frame_done(scene_output, &now);
}