#ifndef _STATE_H_
#define _STATE_H_

#include "zlib.h"
#include "SDL.h"
#include <stdbool.h>

SDL_Surface *state_img;

#define STREAD  0
#define STWRITE 1

#define ST_VER1 1
#define ST_VER2 2
#define ST_VER3 3

Uint8 state_version;
SDL_Surface *load_state_img(char *game,int slot);
bool load_state(char *game,int slot);
bool save_state(char *game,int slot);
Uint32 how_many_slot(char *game);
int mkstate_data(gzFile gzf,void *data,int size,int mode);
void neogeo_init_save_state(void);
void neogeo_cleanup_state();

#endif

