#ifndef VLC_H
#define VLC_H

#include "main.h"
#include "vlc/libvlc.h"
#include "vlc/libvlc_media.h"
#include "vlc/libvlc_media_player.h"

void init_vlc(app_state* state);
void load_file(app_state* state, char* file);
void handle_vlc_events();

#endif