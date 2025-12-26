#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wayland-client.h"
#include "wayland-egl.h"
#include "glad/glad_egl.h"
#include "glad/glad.h"
#include "rendering.h"
#include "listeners.h"
#include "main.h"

void init_egl(app_state* state) {
    egl_window = wl_egl_window_create(state->wl_surface, window_width, window_height);
    egl_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, state->wl_display, NULL);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        cleanup(state, 1);
    }
    if (!eglInitialize(egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        cleanup(state, 1);
    }

    const EGLint win_attrib[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLint num_config;
    if (!eglChooseConfig(egl_display, win_attrib, egl_config, 1, &num_config)) {
        fprintf(stderr, "Failed to set EGL frame buffer config\n");
        cleanup(state, 1);
    }

    eglBindAPI(EGL_OPENGL_API);

    // Check for OpenGL compatibility for creating egl context
    static const struct { int major, minor; } gl_versions[] = {
        {4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
        {3, 3}, {3, 2}, {3, 1}, {3, 0},
        {0, 0}
    };
    egl_context = NULL;
    for (uint32_t i=0; gl_versions[i].major > 0; i++) {
        const EGLint ctx_attrib[] = {
            EGL_CONTEXT_MAJOR_VERSION, gl_versions[i].major,
            EGL_CONTEXT_MINOR_VERSION, gl_versions[i].minor,
            EGL_NONE
        };
        egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, ctx_attrib);
        if (egl_context) {
            fprintf(stderr, "OpenGL %i.%i EGL context created\n", gl_versions[i].major, gl_versions[i].minor);
        }
    }
    if (!egl_context) {
        fprintf(stderr, "Failed to create EGL context\n");
        cleanup(state, 1);
    }

    egl_surface = eglCreateWindowSurface(egl_display, egl_config, egl_window, NULL);

    if (!eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, egl_context)) {
        fprintf(stderr, "Failed to make context current\n");
        cleanup(state, 1);
    }

    if (!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
        fprintf(stderr, "Failed to load OpenGL\n");
        cleanup(state, 1);
    }
}

void create_layer(app_state* state) {
    wl_display_roundtrip(state->wl_display);
    state->wl_surface = wl_compositor_create_surface(state->wl_compositor);
    state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(state->layer_shell, state->wl_surface, state->wl_output, 0, "wlpaper");
    zwlr_layer_surface_v1_add_listener(state->layer_surface, &layer_surface_listener, &state);
    wl_display_roundtrip(state->wl_display);

    zwlr_layer_surface_v1_set_anchor(state->layer_surface,ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(state->layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
}

void draw() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void destroy_layer(app_state* state) {
    zwlr_layer_surface_v1_destroy(state->layer_surface);
}