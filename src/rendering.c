#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "wayland-client.h"
#include "wayland-egl.h"
#include "glad/glad_egl.h"
#include "glad/glad.h"
#include "rendering.h"
#include "listeners.h"
#include "main.h"

void init_egl(app_state* state) {
    if (!strstr(eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS), "EGL_KHR_platform_wayland")) {
        fprintf(stderr, "EGL_KHR_platform_wayland not supported\n");
        cleanup(state, 1);
    }
    state->egl_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, state->wl_display, NULL);
    if (state->egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        cleanup(state, 1);
    }
    if (!eglInitialize(state->egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        cleanup(state, 1);
    }
    
    const EGLint win_attrib[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    EGLint num_config;
    if (!eglChooseConfig(state->egl_display, win_attrib, &state->egl_config, 1, &num_config)) {
        fprintf(stderr, "Failed to set EGL frame buffer config\n");
        cleanup(state, 1);
    }
    
    eglBindAPI(EGL_OPENGL_ES_API);
    
    state->egl_context = NULL;
    const EGLint ctx_attrib[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    state->egl_context = eglCreateContext(state->egl_display, state->egl_config, EGL_NO_CONTEXT, ctx_attrib);
    if (state->egl_context) {
        fprintf(stderr, "OpenGL ES %i EGL context created\n", 2);
    }
    
    if (!state->egl_context) {
        fprintf(stderr, "Failed to create EGL context\n");
        cleanup(state, 1);
    }
    
    state->egl_window = wl_egl_window_create(state->wl_surface, state->window_width, state->window_height);
    if (state->egl_window) {
        state->egl_surface = eglCreatePlatformWindowSurface(state->egl_display, state->egl_config, state->egl_window, NULL);

        if (!state->egl_surface) {
            fprintf(stderr, "Failed to create surface %#x\n", eglGetError());
        }
    } else {
        fprintf(stderr, "Failed to create egl window %#x\n", eglGetError());
    }
    
    if (!eglMakeCurrent(state->egl_display, state->egl_surface, state->egl_surface, state->egl_context)) {
        fprintf(stderr, "Failed to make context current\n");
        cleanup(state, 1);
    }
    
    if (!gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress)) {
        fprintf(stderr, "Failed to load OpenGL\n");
        cleanup(state, 1);
    }
}

void create_layer(app_state* state) {
    wl_display_roundtrip(state->wl_display);
    state->wl_surface = wl_compositor_create_surface(state->wl_compositor);
    wl_surface_set_buffer_scale(state->wl_surface, 1);
    struct wl_callback* wl_surface_cb = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(wl_surface_cb, &wl_surface_frame_cb_listener, state);

    state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(state->layer_shell, state->wl_surface, state->wl_output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wlpaper");
    zwlr_layer_surface_v1_add_listener(state->layer_surface, &layer_surface_listener, state);
    
    zwlr_layer_surface_v1_set_anchor(state->layer_surface,ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(state->layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
    zwlr_layer_surface_v1_set_size(state->layer_surface, 0, 0);

    wl_surface_commit(state->wl_surface);
}

void draw(app_state* state) {
    glViewport(0, 0, state->window_width, state->window_height);
    srand((unsigned int)time(NULL));
    float random_float_r = (float)rand() / (float)RAND_MAX;
    float random_float_g = (float)rand() / (float)RAND_MAX;
    float random_float_b = (float)rand() / (float)RAND_MAX;
    glClearColor(random_float_r, random_float_g, random_float_b, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void destroy_layer(app_state* state) {
    IF_EXISTS_THEN(state->layer_surface, zwlr_layer_surface_v1_destroy(state->layer_surface));
    IF_EXISTS_THEN(state->layer_shell, zwlr_layer_shell_v1_destroy(state->layer_shell));
    IF_EXISTS_THEN(state->wl_surface, wl_surface_destroy(state->wl_surface));
}