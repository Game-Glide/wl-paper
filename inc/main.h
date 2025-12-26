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
    struct rendering_state* render_state;
} typedef app_state;

struct rendering_state {
    EGLContext* egl_context;
    EGLSurface* egl_surface;
    EGLDisplay egl_display;
    EGLConfig* egl_config;

    uint32_t width, height, scale;
} typedef rendering_state;

void cleanup(app_state* state);

#endif