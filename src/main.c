#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "wayland-client.h"
#include "main.h"
#include "listeners.h"
#include "rendering.h"

static app_state state = { 0, .is_first_configure = 1 };
struct wl_egl_window* egl_window;
EGLContext* egl_context;
EGLSurface* egl_surface;
EGLDisplay egl_display;
EGLConfig* egl_config;
uint32_t window_width, window_height, window_scale;

static uint8_t running = 1;

void stop_running(int signum) {
    fprintf(stdout, "\nexiting cleanly.\n");
    running = 0;
    _Exit(signum);
}

int main() {
    state.wl_display = wl_display_connect(NULL);
    if (!state.wl_display) {
        fprintf(stderr, "Failed to establish connection with display\n");
        return 1;
    }
    printf("Established connection with wayland display\n");
    state.wl_registry = wl_display_get_registry(state.wl_display);
    wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
    wl_display_roundtrip(state.wl_display);
    create_layer(&state);
    init_egl(&state);
    wl_display_roundtrip(state.wl_display);
    signal(SIGINT, &stop_running);
    
    while (running) {
        wl_display_dispatch(state.wl_display);
    }

    cleanup(&state, 0);
    return 0;
}

void cleanup(app_state* state, uint32_t exit_status) {
    destroy_layer(state);
    wl_surface_destroy(state->wl_surface);
    wl_registry_destroy(state->wl_registry);
    wl_output_destroy(state->wl_output);
    wl_display_disconnect(state->wl_display);

    exit(exit_status);
}