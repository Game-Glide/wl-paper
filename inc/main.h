#ifndef MAIN_H
#define MAIN_H
#include <wayland-client.h>
#include <EGL/egl.h>
#include "wlr-layer-shell-unstable-v1.h"

struct app_state {
    struct wl_display* wl_display;
    struct wl_registry* wl_registry;
    struct wl_compositor* wl_compositor;
    struct wl_surface* wl_surface;
    struct wl_output* wl_output;
    struct zwlr_layer_shell_v1* layer_shell;
    struct zwlr_layer_surface_v1* layer_surface;

    uint8_t is_first_configure;
} typedef app_state;

extern struct wl_egl_window* egl_window;
extern EGLContext* egl_context;
extern EGLSurface* egl_surface;
extern EGLDisplay egl_display;
extern EGLConfig* egl_config;
extern uint32_t window_width, window_height, window_scale;

void cleanup(app_state* state, uint32_t exit_status);

#endif