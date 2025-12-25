#include <stdio.h>
#include "main.h"
#include "listeners.h"
#include "wayland-client.h"
#include "EGL/egl.h"
#include "GL/gl.h"

int main() {
    app_state state = { 0 };
    state.wl_display = wl_display_connect(NULL);
    if (!state.wl_display) {
        fprintf(stderr, "Failed to establish connection with display\n");
        return 1;
    }
    printf("Established connection with wayland display");
    state.wl_registry = wl_display_get_registry(state.wl_display);
    wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
    wl_display_roundtrip(state.wl_display);
    state.wl_surface = wl_compositor_create_surface(state.wl_compositor);

    cleanup(&state);
    return 0;
}

void cleanup(app_state* state) {
    wl_surface_destroy(state->wl_surface);
    wl_registry_destroy(state->wl_registry);
    wl_display_disconnect(state->wl_display);
}