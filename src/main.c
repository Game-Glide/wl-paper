#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include "wayland-client.h"
#include "main.h"
#include "listeners.h"
#include "rendering.h"
#include "mpv.h"

static app_state state = { 0, .is_egl_ready = false, .running = 1 };

void stop_running(int signum) {
    (void)signum;
    state.running = 0;
}

int main() {
    state.wl_display = wl_display_connect(NULL);
    if (!state.wl_display) {
        fprintf(stderr, "Failed to establish connection with display\n");
        return 1;
    }
    printf("Established connection with wayland display\n");
    state.wl_registry = wl_display_get_registry(state.wl_display);
    wl_registry_add_listener(state.wl_registry, &registry_listener, &state);
    wl_display_roundtrip(state.wl_display);
    create_layer(&state);
    wl_display_roundtrip(state.wl_display);

    struct sigaction sa = {0};
    sa.sa_handler = stop_running;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    int wl_fd = wl_display_get_fd(state.wl_display);

    while (state.running) {
        handle_mpv_events(&state);

        wl_display_dispatch_pending(state.wl_display);
        wl_display_flush(state.wl_display);

        struct pollfd pfds[2] = {
            {
                .fd = wl_fd,
                .events = POLLIN
            },
            {
                .fd = state.mpv_fd,
                .events = POLLIN
            }
        };

        int ret = poll(pfds, 2, 5);
        if (ret == -1) {
            if (errno == EINTR && !state.running)
                break;
            perror("poll");
            break;
        }

        if (pfds[0].revents & POLLIN) {
            if (wl_display_dispatch(state.wl_display) == -1) {
                fprintf(stderr, "Wayland connection lost\n");
                break;
            }
        }

        if (pfds[1].revents & POLLIN) {
            handle_mpv_events(&state);
        }

        if (pfds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "MPV fd error\n");
            break;
        }

        if (pfds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) {
            fprintf(stderr, "Wayland fd error\n");
            break;
        }
    }

    cleanup(&state, 0);
}

void cleanup(app_state* state, uint32_t exit_status) {
    // MPV Resources
    IF_EXISTS_THEN(state->mpv_ctx, mpv_render_context_free(state->mpv_ctx));
    IF_EXISTS_THEN(state->mpv, mpv_terminate_destroy(state->mpv));

    // EGL Resources
    if (state->egl_display) {
        eglMakeCurrent(state->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    IF_EXISTS_THEN(state->egl_context, eglDestroyContext(state->egl_display, state->egl_context));
    IF_EXISTS_THEN(state->egl_surface, eglDestroySurface(state->egl_display, state->egl_surface));
    IF_EXISTS_THEN(state->egl_window, wl_egl_window_destroy(state->egl_window));
    IF_EXISTS_THEN(state->egl_display, eglTerminate(state->egl_display));

    // Free WL Resources
    destroy_layer(state);
    IF_EXISTS_THEN(state->wl_output, wl_output_destroy(state->wl_output));
    IF_EXISTS_THEN(state->wl_compositor, wl_compositor_destroy(state->wl_compositor));
    IF_EXISTS_THEN(state->wl_registry, wl_registry_destroy(state->wl_registry));
    IF_EXISTS_THEN(state->wl_display, wl_display_disconnect(state->wl_display));

    exit(exit_status);
}