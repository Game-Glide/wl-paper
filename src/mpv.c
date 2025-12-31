#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>
#include <stdio.h>
#include "listeners.h"
#include "mpv.h"

static void* get_proc_address(void *ctx, const char *name) {
    return (void*)(uintptr_t)eglGetProcAddress(name);
}

static void mpv_render_update_cb(void *ctx) {
    app_state* state = ctx;
    state->needs_redraw = true;
}

void init_mpv(app_state* state) {
    state->mpv = mpv_create();
    if (state->mpv == NULL) {
        fprintf(stderr, "Failed to create mpv handle");
    }

    mpv_set_option_string(state->mpv, "loop", "inf");
    mpv_set_option_string(state->mpv, "panscan", "1.0");
    mpv_set_option_string(state->mpv, "video-sync", "display-resample");
    mpv_set_option_string(state->mpv, "video-unscaled", "no");
    mpv_set_option_string(state->mpv, "keepaspect", "no");
    mpv_set_option_string(state->mpv, "vo", "libmpv");
    mpv_set_option_string(state->mpv, "hwdec", "auto");
    mpv_set_option_string(state->mpv, "opengl-es", "yes");
    mpv_set_option_string(state->mpv, "gpu-api", "opengl");
    mpv_set_option_string(state->mpv, "log-file", "./logs/mpv.log");

    int err = mpv_initialize(state->mpv);

    if (err < 0) {
        fprintf(stderr, "failed mpv_initialize\n");
        cleanup(state, 1);
    }

    mpv_render_param params[] = {
        { MPV_RENDER_PARAM_API_TYPE, (void*)MPV_RENDER_API_TYPE_OPENGL },
        { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &(mpv_opengl_init_params){
            .get_proc_address = &get_proc_address,
        }},
        { MPV_RENDER_PARAM_INVALID, NULL }
    };

    err = mpv_render_context_create(&state->mpv_ctx, state->mpv, params);
    if (err < 0) {
        fprintf(stderr, "failed mpv_render_context_create\n");
        cleanup(state, 1);
    }
    mpv_render_context_set_update_callback(state->mpv_ctx, mpv_render_update_cb, state);
    state->mpv_fd = mpv_get_wakeup_pipe(state->mpv);
    if (state->mpv_fd < 0) {
        fprintf(stderr, "mpv_get_wakeup_pipe failed\n");
        cleanup(state, 1);
    }
}

void load_file(app_state* state, char* filename) {
    const char* cmd[] = {
        "loadfile",
        filename,
        NULL
    };

    int err = mpv_command(state->mpv, cmd);
    if (err < 0) {
        printf("loading video %d\n", err);
        cleanup(state, 1);
    }
}

void handle_mpv_events(app_state *state) {
    while (1) {
        mpv_event *event = mpv_wait_event(state->mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;

        switch (event->event_id) {
            case MPV_EVENT_LOG_MESSAGE: {
                mpv_event_log_message *msg = event->data;
                fprintf(stderr, "[mpv][%s] %s", msg->level, msg->text);
                break;
            }

            case MPV_EVENT_VIDEO_RECONFIG:
                fprintf(stderr, "[mpv] video reconfigured\n");
                break;

            case MPV_EVENT_START_FILE:
                fprintf(stderr, "[mpv] start file\n");
                break;

            case MPV_EVENT_FILE_LOADED:
                fprintf(stderr, "[mpv] file loaded\n");
                break;

            case MPV_EVENT_END_FILE: {
                mpv_event_end_file *e = event->data;
                fprintf(stderr, "[mpv] end file, reason=%d\n", e->reason);
                break;
            }

            case MPV_EVENT_SHUTDOWN:
                fprintf(stderr, "[mpv] shutdown requested\n");
                state->running = 0;
                break;

            default:
                break;
        }
    }
}