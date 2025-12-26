#include <stdio.h>
#include "wlr-layer-shell-unstable-v1.h"
#include "wayland-client.h"
#include "EGL/egl.h"
#include "main.h"
#include "listeners.h"
#include "rendering.h"

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
    create_layer(&state);
    init_egl(&state);

    while (1) {
        wl_display_dispatch(state.wl_display);
    }

    cleanup(&state);
    return 0;
}

void cleanup(app_state* state) {
    destroy_layer(state);
    wl_surface_destroy(state->wl_surface);
    wl_registry_destroy(state->wl_registry);
    wl_output_destroy(state->wl_output);
    wl_display_disconnect(state->wl_display);
}