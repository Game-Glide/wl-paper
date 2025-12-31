#include <stdbool.h>
#include <stdio.h>
#include <string.h> 
#include "wayland-client.h"
#include "wayland-egl.h"
#include "glad/glad_egl.h"
#include "glad/glad.h"
#include "listeners.h"
#include "rendering.h"
#include "mpv.h"
#include "main.h"

const struct wl_registry_listener registry_listener = {
    .global = handle_global_bind,
    .global_remove = handle_global_remove
};

const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = handle_layer_surface_configure,
    .closed = handle_layer_surface_closed,
};

const struct wl_callback_listener wl_surface_frame_cb_listener = {
    .done = wl_surface_frame_done
};

void handle_global_bind(void* data, struct wl_registry* wl_registry, uint32_t name, const char *interface, uint32_t version) {
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
        init_mpv(state);
        load_file(state, state->filename);

        if (!state->frame_callback) {
            state->needs_redraw = true;
            wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
            
            state->frame_callback = wl_surface_frame(state->wl_surface);
            wl_callback_add_listener(state->frame_callback, &wl_surface_frame_cb_listener, state);
            wl_surface_commit(state->wl_surface);
        }
    
        state->is_egl_ready = true;
    } else {
        printf("resizing window\n");
        wl_egl_window_resize(
            state->egl_window,
            state->window_width,
            state->window_height,
            0, 0
        );
        if (!state->egl_surface) {
            fprintf(stderr, "Failed to create surface %#x\n", eglGetError());
        }
        eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);

        if (!state->frame_callback) {
            state->needs_redraw = true;

            wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
            if(!eglSwapBuffers(state->egl_display, state->egl_surface)) {
                fprintf(stderr, "Failed to swap buffers %#x\n", eglGetError());
            }
            mpv_render_context_report_swap(state->mpv_ctx);
            wl_surface_commit(state->wl_surface);
        }

    }
}

void handle_layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *zwlr_layer_surface_v1) {
    app_state* state = data;
    destroy_layer(state);
}

void wl_surface_frame_done(void* data, struct wl_callback* cb, uint32_t time) {
    app_state* state = data;
    wl_callback_destroy(cb);
    state->frame_callback = NULL;
    
    if (state->needs_redraw) {
        eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context);

        draw(state);

        wl_surface_damage_buffer(state->wl_surface, 0, 0, INT32_MAX, INT32_MAX);

        if (!eglSwapBuffers(state->egl_display, state->egl_surface)) {
            fprintf(stderr, "eglSwapBuffers failed %#x\n", eglGetError());
        }
        mpv_render_context_report_swap(state->mpv_ctx);
        state->needs_redraw = false;
    }
    
    state->frame_callback = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(
        state->frame_callback,
        &wl_surface_frame_cb_listener,
        state
    );
    wl_surface_commit(state->wl_surface);
}
