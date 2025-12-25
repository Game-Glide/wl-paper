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
    struct zwlr_layer_shell_v1* layer_shell;
    struct EGLSurface* egl_surface;
} typedef app_state;

void cleanup(app_state* state);

#endif