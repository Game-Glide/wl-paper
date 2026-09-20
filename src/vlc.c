#include "vlc.h"
#include "main.h"

void init_vlc(app_state* state) {
    state->vlc_instance = libvlc_new(0, NULL);
    if (state->vlc_instance == NULL) {
        fprintf(stderr, "Failed to create vlc instance");
    }
}