#ifndef RENDERING_H
#define RENDERING_H
#include "main.h"


void init_egl(app_state* state);
void create_layer(app_state* state);
void destroy_layer(app_state* state);
void draw(app_state* state);

#endif