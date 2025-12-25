#include "listeners.h"
#include "main.h"
#include "stdio.h"
#include "string.h"

void handle_global_bind(void* data, struct wl_registry* wl_registry, uint32_t name, const char *interface, uint32_t version) {
    printf("interface: %s, name: %d, version: %d\n", interface, name, version);
    app_state* state = data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        state->wl_compositor = wl_registry_bind(wl_registry, name, &wl_compositor_interface, 6);
    }
    else if (strcmp(interface, wl_surface_interface.name)) {
        state->layer_shell = wl_registry_bind(wl_registry, name, &zwlr_layer_shell_v1_interface.name, 5);
    }
}

void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
    // idrk tbh
}