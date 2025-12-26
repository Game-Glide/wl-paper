#include <stdlib.h>
#include <stdio.h>
#include "wayland-client.h"
#include "glad/glad_egl.h"
#include "glad/glad.h"
#include "rendering.h"
#include "listeners.h"
#include "main.h"

void init_egl(app_state* state) {
    state->render_state->egl_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, state->wl_display, NULL);
    if (state->render_state->egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display");
        cleanup(state);
        exit(1);  
    }
    if (!eglInitialize(state->render_state->egl_display, NULL, NULL)) {
        fprintf(stderr, "Failed to initialize EGL");
        cleanup(state);
        exit(1);
    }

    eglBindAPI(EGL_OPENGL_API);
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
    if (!eglChooseConfig(state->render_state->egl_display, win_attrib, state->render_state->egl_config, 1, &num_config)) {
        fprintf(stderr, "Failed to set EGL frame buffer config");
        cleanup(state);
        exit(1);
    }

    // Check for OpenGL compatibility for creating egl context
    static const struct { int major, minor; } gl_versions[] = {
        {4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
        {3, 3}, {3, 2}, {3, 1}, {3, 0},
        {0, 0}
    };
    state->render_state->egl_context = NULL;
    for (uint32_t i=0; gl_versions[i].major > 0; i++) {
        const EGLint ctx_attrib[] = {
            EGL_CONTEXT_MAJOR_VERSION, gl_versions[i].major,
            EGL_CONTEXT_MINOR_VERSION, gl_versions[i].minor,
            EGL_NONE
        };
        state->render_state->egl_context = eglCreateContext(state->render_state->egl_display, state->render_state->egl_config, EGL_NO_CONTEXT, ctx_attrib);
        if (state->render_state->egl_context) {
            fprintf(stderr, "OpenGL %i.%i EGL context created", gl_versions[i].major, gl_versions[i].minor);
        }
    }
    if (!state->render_state->egl_context) {
        fprintf(stderr, "Failed to create EGL context");
        cleanup(state);
        exit(1);
    }

    if (!eglMakeCurrent(state->render_state->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, state->render_state->egl_context)) {
        fprintf(stderr, "Failed to make context current");
        cleanup(state);
        exit(1);
    }

    if (!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
        fprintf(stderr, "Failed to load OpenGL");
        cleanup(state);
        exit(1);
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

void draw(app_state* state) {
    glViewport(0, 0, state->render_state->width, state->render_state->width);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0, 1.0, 1.0, 1.0);

    eglSwapBuffers(state->render_state->egl_display, state->render_state->egl_surface);
    wl_surface_commit(state->wl_surface);
}

void destroy_layer(app_state* state) {
    zwlr_layer_surface_v1_destroy(state->layer_surface);
}