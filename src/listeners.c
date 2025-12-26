#include "listeners.h"
#include "rendering.h"
#include "main.h"
#include "stdio.h"
#include "string.h"

void handle_global_bind(void* data, struct wl_registry* wl_registry, uint32_t name, const char *interface, uint32_t version) {
    printf("interface: %s, name: %d, version: %d\n", interface, name, version);
    app_state* state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 6);
    }
    else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        state->layer_shell = wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface, 5);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        state->wl_output = wl_registry_bind(wl_registry, name, &wl_output_interface, 4);
    }
}

void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
    // idrk tbh
}

void handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height) {
    app_state* state = data;
    state->render_state->height = height;
    state->render_state->width = width;

    zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
    wl_surface_commit(state->wl_surface);
    draw(state);
}

void handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1) {
    // Not sure yet again
}
