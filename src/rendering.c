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

#define CASE_STR(value) case value: return #value;
const char *eglGetErrorString(EGLint error) {
    switch (error) {
        CASE_STR(EGL_SUCCESS)
        CASE_STR(EGL_NOT_INITIALIZED)
        CASE_STR(EGL_BAD_ACCESS)
        CASE_STR(EGL_BAD_ALLOC)
        CASE_STR(EGL_BAD_ATTRIBUTE)
        CASE_STR(EGL_BAD_CONTEXT)
        CASE_STR(EGL_BAD_CONFIG)
        CASE_STR(EGL_BAD_CURRENT_SURFACE)
        CASE_STR(EGL_BAD_DISPLAY)
        CASE_STR(EGL_BAD_SURFACE)
        CASE_STR(EGL_BAD_MATCH)
        CASE_STR(EGL_BAD_PARAMETER)
        CASE_STR(EGL_BAD_NATIVE_PIXMAP)
        CASE_STR(EGL_BAD_NATIVE_WINDOW)
        CASE_STR(EGL_CONTEXT_LOST)
    default: return "Unknown Error";
    }
}
#undef CASE_STR

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
            fprintf(stderr, "Failed to create surface %s\n", eglGetErrorString(eglGetError()));
        }
    } else {
        fprintf(stderr, "Failed to create egl window %s\n", eglGetErrorString(eglGetError()));
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
    state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(state->layer_shell, state->wl_surface, state->wl_output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wlpaper");
    zwlr_layer_surface_v1_add_listener(state->layer_surface, &layer_surface_listener, state);
    
    zwlr_layer_surface_v1_set_anchor(state->layer_surface,ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(state->layer_surface, -1);
    zwlr_layer_surface_v1_set_keyboard_interactivity(state->layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_NONE);
    zwlr_layer_surface_v1_set_size(state->layer_surface, 0, 0);

    wl_surface_commit(state->wl_surface);
}

void draw(app_state* state) {
    printf("rendering\n");
    glViewport(0, 0, state->window_width, state->window_height);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if(!eglSwapBuffers(state->egl_display, state->egl_surface)) {
        fprintf(stderr, "Failed to swap buffers %s\n", eglGetErrorString(eglGetError()));
    }
}

void destroy_layer(app_state* state) {
    IF_EXISTS_THEN(state->layer_surface, zwlr_layer_surface_v1_destroy(state->layer_surface));
    IF_EXISTS_THEN(state->layer_shell, zwlr_layer_shell_v1_destroy(state->layer_shell));
    IF_EXISTS_THEN(state->wl_surface, wl_surface_destroy(state->wl_surface));
}