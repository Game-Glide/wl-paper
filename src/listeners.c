#include <stdbool.h>
#include <stdio.h>
#include <string.h> 
#include "wayland-client.h"
#include "wayland-egl.h"
#include "glad/glad_egl.h"
#include "glad/glad.h"
#include "listeners.h"
#include "rendering.h"
#include "main.h"

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
    if (width == 0 || height == 0) {
        zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);
        return;
    }
    app_state* state = data;
    state->window_height = height;
    state->window_width = width;

    zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);

    if (!state->is_egl_ready) {
        init_egl(state);

        draw(state);
    
        wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
        if(!eglSwapBuffers(state->egl_display, state->egl_surface)) {
            fprintf(stderr, "Failed to swap buffers %#x\n", eglGetError());
        }
        state->is_egl_ready = true;
    } else {
        printf("resizing window\n");
        eglDestroySurface(state->egl_display, state->egl_surface);
        wl_egl_window_resize(
            state->egl_window,
            state->window_width,
            state->window_height,
            0, 0
        );
        state->egl_surface = eglCreateWindowSurface(state->egl_display, state->egl_config, state->egl_window, NULL);
        eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);

        draw(state);
        
        wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
        if(!eglSwapBuffers(state->egl_display, state->egl_surface)) {
            fprintf(stderr, "Failed to swap buffers %#x\n", eglGetError());
        }
    }
}

void handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1) {
    app_state* state = data;
    destroy_layer(state);
}

void wl_surface_frame_done(void* data, struct wl_callback* cb, uint32_t time) {
    wl_callback_destroy(cb);

    app_state* state = data;
    cb = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_cb_listener, state);

    draw(state);
    
    if(!eglSwapBuffers(state->egl_display, state->egl_surface)) {
        fprintf(stderr, "Failed to swap buffers %#x\n", eglGetError());
    }

    wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
    wl_surface_commit(state->wl_surface);
}
