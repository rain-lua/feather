#include "../include/defines.hpp"
#include "../debug/debug.hpp"
#include "./types/structs.hpp"
#include "./events/events.hpp"

int main(int argc, char **argv){
	if (!getenv("XDG_RUNTIME_DIR")) {
		log_error("XDG_RUNTIME_DIR not set, cannot create Wayland socket");
		return 1;
	}

    struct fwm_server server = {0};

    server.wl_display = wl_display_create();

    server.backend = wlr_backend_autocreate(wl_display_get_event_loop(server.wl_display), NULL);
    if (server.backend == NULL) {
        log_error("failed to create wlr_backend");
        return 1;
    }

    server.renderer = wlr_renderer_autocreate(server.backend);
    if (server.renderer == NULL) {
        log_error("failed to create wlr_renderer");
        return 1;
    }

    wlr_renderer_init_wl_display(server.renderer, server.wl_display);

    server.allocator = wlr_allocator_autocreate(server.backend, server.renderer);
    if (server.allocator == NULL) {
        log_error("failed to create wlr_allocator");
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

    log_info("fwm running on WAYLAND_DISPLAY=%s", socket);
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