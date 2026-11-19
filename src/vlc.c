#include "vlc/libvlc_media.h"
#include "vlc.h"
#include "main.h"

void init_vlc(app_state* state) {
    const char *argv[] = {
        "--no-xlib", 
        "--no-video-title-show"
    };
    int argc = sizeof(argv) / sizeof(*argv);

    state->vlc_instance = libvlc_new(argc, argv);
    if (state->vlc_instance == NULL) {
        fprintf(stderr, "Failed to create vlc instance");
    }
}

void load_file(app_state* state, char* filename) {
    state->vlc_media_player = libvlc_media_player_new(state->vlc_instance);
    int media_fd = libvlc_media_new_path(state->vlc_instance, filename);
    libvlc_media_add_option(media_fd, ":input-repeat=65535"); // Looping video
    libvlc_media_player_set_media(state->vlc_media_player, media_fd);

    libvlc_media_release(media_fd);
    libvlc_media_player_play(state->vlc_media_player);
}