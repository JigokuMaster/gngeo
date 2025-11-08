#ifndef RESFILE_H
#define RESFILE_H

#include "SDL.h"
#include "roms.h"
#include "stb_image.h"

typedef struct RESDATA
{
  stbi_uc *data;
  SDL_Surface *img;
}RESDATA;

ROM_DEF *res_load_drv(char *name);
SDL_Surface *res_load_bmp(char *bmp);
void *res_load_data(char *name);
//SDL_Surface *res_load_stbi(char *bmp);
RESDATA* res_load_stbi(char *bmp, SDL_Surface **s);
void res_free_data(RESDATA* rd);
#endif
