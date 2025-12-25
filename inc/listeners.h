#ifndef LISTENER_H
#define LISTENER_H
#include "wayland-client.h"

void handle_global_bind(void* data, struct wl_registry* wl_registry, uint32_t name, const char *interface, uint32_t version);
void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name);

const static struct wl_registry_listener registry_listener = {
    .global = handle_global_bind,
    .global_remove = handle_global_remove
};

#endif