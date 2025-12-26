#ifndef LISTENER_H
#define LISTENER_H
#include "wayland-client.h"
#include "wlr-layer-shell-unstable-v1.h"

void handle_global_bind(void* data, struct wl_registry* wl_registry, uint32_t name, const char *interface, uint32_t version);
void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);

void handle_layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1, uint32_t serial, uint32_t width, uint32_t height);
void handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1);

const static struct wl_registry_listener registry_listener = {
    .global = handle_global_bind,
    .global_remove = handle_global_remove
};

const static struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = handle_layer_surface_configure,
    .closed = handle_layer_surface_closed,
};

#endif