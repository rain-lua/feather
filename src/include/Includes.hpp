#pragma once

#include <getopt.h>
#include <iostream>
#include <libinput.h>
#include <linux/input-event-codes.h>
#include <wayland-server-core.h>
#include <xkbcommon/xkbcommon.h>

// https://github.com/swaywm/wlroots/issues/682

#define class     wlroots_class
#define namespace wlroots_namespace
#define static

extern "C" {
    #include <wlr/backend.h>
    #include <wlr/render/allocator.h>
    #include <wlr/render/wlr_renderer.h>
    #include <wlr/types/wlr_compositor.h>
    #include <wlr/types/wlr_cursor.h>
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
    #include <wlr/types/wlr_xdg_decoration_v1.h>
    #include <wlr/types/wlr_xdg_shell.h>
    #include <wlr/util/log.h>
    #include <wlr/xwayland.h>
}

#undef class
#undef namespace
#undef static