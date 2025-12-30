#ifndef MPV_H
#define MPV_H
#include <main.h>

void init_mpv(app_state* state);
void load_file(app_state* state, char* filename);
void handle_mpv_events(app_state* state);

#endif