/*  gngeo, a neogeo emulator
 *  Copyright (C) 2001 Peponas Thomas & Peponas Mathieu
 * 
 *  This program is free software; you can redistribute it and/or modify  
 *  it under the terms of the GNU General Public License as published by   
 *  the Free Software Foundation; either version 2 of the License, or    
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *i

 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 
 */

//#define SYMBIAN 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SDL.h"
#include "SDL_thread.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <math.h>

#include "menu.h"

#include "messages.h"
#include "memory.h"
#include "screen.h"
#include "video.h"
#include "event.h"
#include "state.h"
#include "emu.h"
#include "frame_skip.h"
#include "video.h"
#include "conf.h"
#include "resfile.h"
#include "fileio.h"
#include "sound.h"
#include "effect.h"
#ifdef SYMBIAN
#include "symbian_s60.h"
static char audio_vol[32];
#endif

#if defined (WII)
#define ROOTPATH "sd:/apps/gngeo/"
#elif defined (__AMIGA__)
#define ROOTPATH "/PROGDIR/data/"
#else
#define ROOTPATH ""
#endif

typedef struct GNFONT {
	SDL_Surface *bmp;
	Uint8 ysize;
	Uint16 xpos[128 - 32];
	Uint8 xsize[128 - 32];
	Sint8 pad;
} GNFONT;

static SDL_Surface *menu_buf = NULL;
static SDL_Surface *menu_back = NULL;
static SDL_Surface *back = NULL;
static GNFONT *sfont = NULL;
static GNFONT *mfont = NULL;
static SDL_Surface *gngeo_logo, *gngeo_mask, *pbar_logo;
static SDL_Surface *arrow_l, *arrow_r, *arrow_u, *arrow_d;

static int interp;
static int menu_anim = 1;
static char z80_clock[4];
static char m68k_clock[4];

#define MENU_BIG   0
#define MENU_SMALL 1

#define MENU_TITLE_X (24 + 30)
#define MENU_TITLE_Y (16 + 20)

#define MENU_TEXT_X (24 + 26)
#define MENU_TEXT_Y (16 + 43)

#define MENU_TEXT_X_END (24 + 277)
#define MENU_TEXT_Y_END (16 + 198)

#define ALIGN_LEFT   -1
#define ALIGN_RIGHT  -2
#define ALIGN_CENTER -3
#define ALIGN_UP   -1
#define ALIGN_DOWN  -2

#define MENU_CLOSE        1
#define MENU_STAY         0
#define MENU_EXIT         2
#define MENU_RETURNTOGAME 3


static GN_MENU *main_menu = NULL;
static GN_MENU *rbrowser_menu = NULL;
static GN_MENU *option_menu = NULL;
static GN_MENU *effect_menu = NULL;
static GN_MENU *srate_menu  = NULL;
static GN_MENU *yesno_menu  = NULL;

static char *romlist[] = {
	"2020bb",
	"2020bba",
	"2020bbh",
	"3countb",
	"alpham2",
	"androdun",
	"aodk",
	"aof",
	"aof2",
	"aof2a",
	"aof3",
	"aof3k",
	"bakatono",
	"bangbead",
	"bjourney",
	"blazstar",
	"breakers",
	"breakrev",
	"bstars",
	"bstars2",
	"burningf",
	"burningfh",
	"crsword",
	"ct2k3sa",
	"ct2k3sp",
	"cthd2003",
	"ctomaday",
	"cyberlip",
	"diggerma",
	"doubledr",
	"eightman",
	"fatfursa",
	"fatfursp",
	"fatfury1",
	"fatfury2",
	"fatfury3",
	"fbfrenzy",
	"fightfev",
	"fightfeva",
	"flipshot",
	"fswords",
	"galaxyfg",
	"ganryu",
	"garou",
	"garoubl",
	"garouo",
	"garoup",
	"ghostlop",
	"goalx3",
	"gowcaizr",
	"gpilots",
	"gpilotsh",
	"gururin",
	"irrmaze",
	"janshin",
	"jockeygp",
	"joyjoy",
	"kabukikl",
	"karnovr",
	"kf10thep",
	"kf2k2mp",
	"kf2k2mp2",
	"kf2k2pla",
	"kf2k2pls",
	"kf2k3bl",
	"kf2k3bla",
	"kf2k3pcb",
	"kf2k3pl",
	"kf2k3upl",
	"kf2k5uni",
	"kizuna",
	"kof10th",
	"kof2000",
	"kof2000n",
	"kof2001",
	"kof2001h",
	"kof2002",
	"kof2002b",
	"kof2003",
	"kof2003h",
	"kof2k4se",
	"kof94",
	"kof95",
	"kof95h",
	"kof96",
	"kof96h",
	"kof97",
	"kof97a",
	"kof97pls",
	"kof98",
	"kof98k",
	"kof98n",
	"kof99",
	"kof99a",
	"kof99e",
	"kof99n",
	"kof99p",
	"kog",
	"kotm",
	"kotm2",
	"kotmh",
	"lans2004",
	"lastblad",
	"lastbladh",
	"lastbld2",
	"lastsold",
	"lbowling",
	"legendos",
	"lresort",
	"magdrop2",
	"magdrop3",
	"maglord",
	"maglordh",
	"mahretsu",
	"marukodq",
	"matrim",
	"matrimbl",
	"miexchng",
	"minasan",
	"mosyougi",
	"ms4plus",
	"ms5pcb",
	"ms5plus",
	"mslug",
	"mslug2",
	"mslug3",
	"mslug3b6",
	"mslug3h",
	"mslug3n", /* Driver don't have the good name */
	"mslug4",
	"mslug5",
	"mslug5h",
	"mslugx",
	"mutnat",
	"nam1975",
	"ncombat",
	"ncombath",
	"ncommand",
	"neobombe",
	"neocup98",
	"neodrift",
	"neomrdo",
	"ninjamas",
	"nitd",
	"nitdbl",
	"overtop",
	"panicbom",
	"pbobbl2n",
	"pbobblen",
	"pbobblena",
	"pgoal",
	"pnyaa",
	"popbounc",
	"preisle2",
	"pspikes2",
	"pulstar",
	"puzzldpr",
	"puzzledp",
	"quizdai2",
	"quizdais",
	"quizkof",
	"ragnagrd",
	"rbff1",
	"rbff1a",
	"rbff2",
	"rbff2h",
	"rbff2k",
	"rbffspec",
	"ridhero",
	"ridheroh",
	"roboarmy",
	"rotd",
	"s1945p",
	"samsh5sp",
	"samsh5sph",
	"samsh5spn", /* Driver don't have the good name */
	"samsho",
	"samsho2",
	"samsho3",
	"samsho3h",
	"samsho4",
	"samsho5",
	"samsho5b",
	"samsho5h",
	"samshoh",
	"savagere",
	"sdodgeb",
	"sengokh",
	"sengoku",
	"sengoku2",
	"sengoku3",
	"shocktr2",
	"shocktra",
	"shocktro",
	"socbrawl",
	"socbrawla",
	"sonicwi2",
	"sonicwi3",
	"spinmast",
	"ssideki",
	"ssideki2",
	"ssideki3",
	"ssideki4",
	"stakwin",
	"stakwin2",
	"strhoop",
	"superspy",
	"svc",
	"svcboot",
	"svcpcb",
	"svcpcba",
	"svcplus",
	"svcplusa",
	"svcsplus",
	"tophuntr",
	"tophuntra",
	"tpgolf",
	"trally",
	"turfmast",
	"twinspri",
	"tws96",
	"viewpoin",
	"vliner",
	"vlinero",
	"wakuwak7",
	"wh1",
	"wh1h",
	"wh1ha",
	"wh2",
	"wh2j",
	"wh2jh",
	"whp",
	"wjammers",
	"zedblade",
	"zintrckb",
	"zupapa",
	NULL
};

#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))
#if 0
GNFONT *load_font(char *file) {
	GNFONT *ft = malloc(sizeof (GNFONT));
	int i;
	int x = 0;
	Uint32 *b;
	if (!ft) return NULL;
	RESDATA* rd = res_load_stbi(file, &ft->bmp);
	if (!rd)
	{
	    free(ft);
	    return NULL;
	}

	//res_free_data(rd);
	//ft->bmp = res_load_stbi(file);
	if (!ft->bmp)
	{
		free(ft);
		return NULL;
	}
	//SDL_SetAlpha(ft->bmp,SDL_SRCALPHA,128);
	b = ft->bmp->pixels;
	//ft->bmp->format->Amask=0xFF000000;
	//ft->bmp->format->Ashift=24;
	//printf("shift=%d %d\n",ft->bmp->pitch,ft->bmp->w*4);
	if (ft->bmp->format->BitsPerPixel != 32) {
		printf("Unsupported font (bpp=%d)\n", ft->bmp->format->BitsPerPixel);
		SDL_FreeSurface(ft->bmp);
		free(ft);
		return NULL;
	}
	ft->xpos[0] = 0;
	for (i = 0; i < ft->bmp->w; i++) {
		//printf("%08x\n",b[i]);
		if (b[i] != b[0]) {
			ft->xpos[x + 1] = i + 1;
			if (x > 0)
				ft->xsize[x] = i - ft->xpos[x];
			else
				ft->xsize[x] = i;
			x++;
		}
	}
	//printf("NB char found:%d\n",x);
	if (x <= 0 || x > 95) return NULL;
	/*	b=ft->bmp->pixels+ft->bmp->pitch*3;
		for (i=0;i<ft->bmp->w;i++) {
			//printf("%08x\n",b[i]);
		}
	 */
	ft->xsize[94] = ft->bmp->w - ft->xpos[94];
	ft->ysize = ft->bmp->h;

	/* Default y padding=0 */
	ft->pad = 0;
	return ft;
}
#endif

GNFONT *load_font(char *file, RESDATA** rd)
{
	GNFONT *ft = malloc(sizeof (GNFONT));
	int i;
	int x = 0;
	Uint32 *b;
	if (!ft) return NULL;
	*rd = res_load_stbi(file, &ft->bmp);
	if (*rd == NULL)
	{
	    free(ft);
	    return NULL;
	}
	if (!ft->bmp)
	{
		free(ft);
		return NULL;
	}
	//SDL_SetAlpha(ft->bmp,SDL_SRCALPHA,128);
	b = ft->bmp->pixels;
	//ft->bmp->format->Amask=0xFF000000;
	//ft->bmp->format->Ashift=24;
	//printf("shift=%d %d\n",ft->bmp->pitch,ft->bmp->w*4);
	if (ft->bmp->format->BitsPerPixel != 32) {
		printf("Unsupported font (bpp=%d)\n", ft->bmp->format->BitsPerPixel);
		SDL_FreeSurface(ft->bmp);
		free(ft);
		return NULL;
	}
	ft->xpos[0] = 0;
	for (i = 0; i < ft->bmp->w; i++) {
		//printf("%08x\n",b[i]);
		if (b[i] != b[0]) {
			ft->xpos[x + 1] = i + 1;
			if (x > 0)
				ft->xsize[x] = i - ft->xpos[x];
			else
				ft->xsize[x] = i;
			x++;
		}
	}
	//printf("NB char found:%d\n",x);
	if (x <= 0 || x > 95) return NULL;
	/*	b=ft->bmp->pixels+ft->bmp->pitch*3;
		for (i=0;i<ft->bmp->w;i++) {
			//printf("%08x\n",b[i]);
		}
	 */
	ft->xsize[94] = ft->bmp->w - ft->xpos[94];
	ft->ysize = ft->bmp->h;

	/* Default y padding=0 */
	ft->pad = 0;
	return ft;
}


static Uint32 string_len(GNFONT *f, char *str) {
	int i;
	int size = 0;
	if (str) {
		for (i = 0; i < strlen(str); i++) {
			switch (str[i]) {
				case ' ':
					size += f->xsize[0];
					break;
				case '\t':
					size += (f->xsize[0]*8);
					break;
				default:
					size += (f->xsize[(int) str[i] - 32] + f->pad);
					break;
			}
		}
		return size;
	} else
		return 0;
}

void draw_string(SDL_Surface *dst, GNFONT *f, int x, int y, char *str) {
	SDL_Rect srect, drect;
	int i;

	if (x == ALIGN_LEFT) x = MENU_TEXT_X;
	if (x == ALIGN_RIGHT) x = (MENU_TEXT_X_END - string_len(f, str));
	if (x == ALIGN_CENTER) x = (MENU_TEXT_X + (MENU_TEXT_X_END - MENU_TEXT_X - string_len(f, str)) / 2);
	if (y == ALIGN_UP) y = MENU_TEXT_Y;
	if (y == ALIGN_DOWN) y = (MENU_TEXT_Y_END - f->ysize);
	if (y == ALIGN_CENTER) y = (MENU_TEXT_Y + (MENU_TEXT_Y_END - MENU_TEXT_Y - f->ysize) / 2);

	drect.x = x;
	drect.y = y;
	drect.w = 32;
	drect.h = f->bmp->h;
	srect.h = f->bmp->h;
	srect.y = 0;
	for (i = 0; i < strlen(str); i++) {
		switch (str[i]) {
			case ' ': /* Optimize space */
				drect.x += f->xsize[0];
				break;
			case '\t':
				drect.x += (f->xsize[0]*8);
				break;
			case '\n':
				drect.x = x;
				drect.y += f->bmp->h;
				break;
			default:
				srect.x = f->xpos[(int) str[i] - 32];
				srect.w = f->xsize[(int) str[i] - 32];


				SDL_BlitSurface(f->bmp, &srect, dst, &drect);
				drect.x += (f->xsize[(int) str[i] - 32] + f->pad);
				break;
		}
	}
}

static void init_back(void) {
	SDL_Rect dst_r = {24, 16, 304, 224};
	static SDL_Rect screen_rect = {0, 0, 304, 224};
	SDL_BlitSurface(state_img, &screen_rect, menu_back, &dst_r);
	SDL_BlitSurface(back, NULL, menu_back, &dst_r);
}

static void draw_back(void) {
	SDL_Rect dst_r = {24, 16, 304, 224};
	static SDL_Rect screen_rect = {0, 0, 304, 224};
	if (back) {
		//		SDL_BlitSurface(state_img, &screen_rect, menu_buf, &dst_r);
		//		SDL_BlitSurface(back, NULL, menu_buf, &dst_r);
		SDL_BlitSurface(menu_back, NULL, menu_buf, NULL);
	} else {
		SDL_Rect r1 = {24 + 16, 16 + 16, 271, 191};
		SDL_Rect r2 = {24 + 22, 16 + 35, 259, 166};
		SDL_Rect r3 = {24 + 24, 16 + 24, 271, 191};

		SDL_FillRect(menu_buf, &r3, COL32_TO_16(0x111111));
		SDL_FillRect(menu_buf, &r1, COL32_TO_16(0xE8E8E8));
		SDL_FillRect(menu_buf, &r2, COL32_TO_16(0x1C57A2));
	}

}

#define ALEFT  0
#define ARIGHT 1
#define ADOWN  2
#define AUP    3

static void draw_arrow(int type, int x, int y) {
	SDL_Rect dst_r = {x, y - 13, 32, 32};
	switch (type) {
		case ARIGHT:
			SDL_BlitSurface(arrow_r, NULL, menu_buf, &dst_r);
			break;
		case ALEFT:
			SDL_BlitSurface(arrow_l, NULL, menu_buf, &dst_r);
			break;
		case AUP:
			SDL_BlitSurface(arrow_u, NULL, menu_buf, &dst_r);
			break;
		case ADOWN:
			SDL_BlitSurface(arrow_d, NULL, menu_buf, &dst_r);
			break;
	}
}


RESDATA* img_res[7];
RESDATA* font_res[2];
  
int gn_init_skin(void) {
	//menu_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 32, 0xFF0000, 0xFF00, 0xFF, 0x0);
	menu_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 16, 0xF800, 0x7E0, 0x1F, 0);
	//	menu_back= SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 32, 0xFF0000, 0xFF00, 0xFF, 0x0);
	menu_back = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 16, 0xF800, 0x7E0, 0x1F, 0);

	sfont = load_font("skin/font_small.tga", &font_res[0]);
	mfont = load_font("skin/font_large.tga", &font_res[1]);
	img_res[0]  =  res_load_stbi("skin/back.tga", &back);
	img_res[1]  = res_load_stbi("skin/arrow_right.tga", &arrow_r);
	img_res[2]  = res_load_stbi("skin/arrow_left.tga", &arrow_l);
	img_res[3]  = res_load_stbi("skin/arrow_down.tga", &arrow_d);
	img_res[4]  = res_load_stbi("skin/arrow_up.tga", &arrow_u);
	img_res[5]  = res_load_stbi("skin/gngeo.tga", &gngeo_logo);
	img_res[6] =  res_load_stbi("skin/gngeo_mask.tga", &gngeo_mask);

	pbar_logo = SDL_CreateRGBSurface(SDL_SWSURFACE, gngeo_logo->w, gngeo_logo->h, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
	SDL_SetAlpha(gngeo_logo, 0, 0);
	//SDL_SetAlpha(gngeo_mask,0,0);
	init_back();

	if (!back || !sfont || !mfont || !arrow_r || !arrow_l || !arrow_u || !arrow_d ||
			!gngeo_logo || !menu_buf) return false;
	return true;
}

typedef struct pbar_data {
	char *name;
	int pos;
	int size;
	int running;
} pbar_data;

static volatile pbar_data pbar;

#ifdef SYMBIAN

void gn_reset_pbar(void)
{
	pbar.pos = 0;
	pbar.size = 0;
}

void gn_init_pbar(char *name, int size)
{
	pbar.name = name;
	pbar.pos = 0;
	pbar.size = size;
}

void gn_update_pbar(int pos)
{
    char pb_msg[255];
    sprintf(pb_msg, "Loading game ... %d/%d", pos ,  pbar.size);
    draw_back();
    draw_string(menu_buf, sfont, MENU_TITLE_X + 50, MENU_TITLE_Y + 100, pb_msg);
    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_update();
    frame_skip(0);
}

void gn_terminate_pbar(void)
{
    gn_reset_pbar();
    SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);
    screen_update();
    frame_skip(0);
}

#else

static int pbar_y;

void gn_reset_pbar(void) {
	pbar_y = 0;
}


static SDL_Thread *pbar_th;

int pbar_anim_thread(void *data) {
	pbar_data *p = (pbar_data*) data;
	SDL_Rect src_r = {2, 0, gngeo_logo->w, gngeo_logo->h};
	SDL_Rect dst_r = {219 + 26, 146 + 16, gngeo_logo->w, gngeo_logo->h};
	SDL_Rect dst2_r = {0, 0, gngeo_logo->w, gngeo_logo->h};
	int x = 0;

	while (p->running) {
		draw_back();
		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y + 150, p->name);
		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y + 165, "Please wait!");

		SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);


		dst2_r.y = -22 - (p->pos * 64.0) / p->size;
		x += 3;
		if (x > gngeo_logo->w)
			x -= gngeo_logo->w;

		dst2_r.x = x;
		SDL_BlitSurface(gngeo_mask, NULL, pbar_logo, &dst2_r);

		dst2_r.x = x - gngeo_logo->w;
		dst2_r.y = -22 - (p->pos * 64.0) / p->size;
		SDL_BlitSurface(gngeo_mask, NULL, pbar_logo, &dst2_r);



		SDL_BlitSurface(pbar_logo, &src_r, menu_buf, &dst_r);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		//printf("TOTO %d %d %d\n",p->pos,p->size,dst2_r.x);
	}
	SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);
	SDL_BlitSurface(pbar_logo, &src_r, menu_buf, &dst_r);
	screen_update();
	frame_skip(0);
	return 0;
}


void gn_init_pbar(char *name, int size) {
	pbar.name = name;
	pbar.pos = 0;
	pbar.size = size;
	pbar.running = 1;
	pbar_th = SDL_CreateThread(pbar_anim_thread, (void*) &pbar);
}

void gn_update_pbar(int pos) {
	pbar.pos = pos;
}

void gn_terminate_pbar(void) {
	pbar.running = 0;
	SDL_WaitThread(pbar_th, NULL);
}
#endif

void gn_popup_error(char *name, char *fmt, ...) {
	char buf[512];
	va_list pvar;
	va_start(pvar, fmt);

	reset_event();

	draw_back();
	draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, name);

	vsnprintf(buf, 511, fmt, pvar);

	draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);

	draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "Press any key ...");
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();

	while (wait_event() == 0);
}

// FIX: Yes/No dialog actions
static int yes_action(GN_MENU_ITEM *self, void *param) {
	return 2;
}

static int no_action(GN_MENU_ITEM *self, void *param) {
	return 1;
}

/* TODO: use a mini yes/no menu instead of B/X */
int gn_popup_question(char *name, char *fmt, ...) {
	char buf[512];
	va_list pvar;
	va_start(pvar, fmt);
	int a;

	reset_event();

	while (1) {
		draw_back();

		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, name);

		vsnprintf(buf, 511, fmt, pvar);
		draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);
		//draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "   Yes     No");
		if (yesno_menu->current == 0)
			draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, " > Yes     No");
		else
			draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "   Yes  >  No");
		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();

			//effect_menu->draw(effect_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = yesno_menu->event_handling(yesno_menu)) >= 0) {
			printf("return %d\n",a);
			return a;
		}

	}
	return 0;
}

//#define NB_ITEM_2 4

static void draw_menu(GN_MENU *m) {
	int start, end, i;
	static int cx = 0;
	static int cy = 0;
	//static int cx_val[]={0,1,1,2,2,2,1,1,0,-1,-1,-2,-2,-2,-1,-1};
	static int cx_val[] = {0, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, -1, -1, -1, -2, -2, -2, -2, -2, -1, -1, -1, 0};
	int nb_item;
	int ypos, cpos;
	GNFONT *fnt;
	LIST *l = m->item;
	int j;

	if (m->draw_type == MENU_BIG)
		fnt = mfont;
	else
		fnt = sfont;

	/* Arrow moving cursor */
	cx++;
	if (cx > 25) cx = 0;



	nb_item = (MENU_TEXT_Y_END - MENU_TEXT_Y) / fnt->ysize - 1;

	draw_back();

	if (m->title) {
		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, m->title);
	}
	/*
		start = (m->current - nb_item >= 0 ? m->current - nb_item : 0);
		end = (start + nb_item < m->nb_elem ? start + nb_item : m->nb_elem - 1);
	 */
	start = m->current - nb_item / 2;
	end = m->current + nb_item / 2;
	if (start < 0) start = 0;
	if (end >= m->nb_elem - 1) {
		end = m->nb_elem - 1;
	} else {
		draw_arrow(ADOWN, 24, 200 + cx_val[cx]);
	}

	if (m->current <= nb_item / 2) {
		j = nb_item / 2 - m->current;
	} else {
		j = 0;
		draw_arrow(AUP, 24, 76 - cx_val[cx]);
	}

	//printf("%d %d %d %d\n",start,end,m->current,j);
	for (i = 0; i < start; i++, l = l->next) {
		GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
		if (mi->enabled == 0) {
			i--;
		}
	}
	for (i = start; i <= end; i++, l = l->next) {
		GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
		if (mi->enabled == 0) {
			i--;
			continue;
		}
		//int j = (i + nb_item / 2) - start;
		//if (start<nb_item) j+=nb_item/2;
		if (m->draw_type == MENU_BIG) {
			draw_string(menu_buf, fnt, ALIGN_CENTER, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
			if (i == m->current) {
				int len = string_len(fnt, mi->name) / 2;
				draw_arrow(ARIGHT, 176 - len - 32 + cx_val[cx], MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
				draw_arrow(ALEFT, 176 + len - cx_val[cx], MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
			}

		} else {
			draw_string(menu_buf, fnt, MENU_TEXT_X + 10, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
			if (i == m->current) draw_string(menu_buf, fnt, MENU_TEXT_X, MENU_TEXT_Y + (j * fnt->ysize + 2), ">");
			if (mi->type == MENU_CHECK) {
				if (mi->val)
					draw_string(menu_buf, fnt, MENU_TEXT_X + 210, MENU_TEXT_Y + (j * fnt->ysize + 2), "true");
				else
					draw_string(menu_buf, fnt, MENU_TEXT_X + 210, MENU_TEXT_Y + (j * fnt->ysize + 2), "false");
			}
			if (mi->type == MENU_LIST) {
				draw_string(menu_buf, fnt, MENU_TEXT_X + 210, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->str);
			}
		}
		j++;
	}
	SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	screen_update();
	frame_skip(0);
}

//#undef NB_ITEM_2

GN_MENU_ITEM *gn_menu_create_item(char *name, Uint32 type,
		int (*action)(GN_MENU_ITEM *self, void *param), void *param) {
	GN_MENU_ITEM *t = malloc(sizeof (GN_MENU_ITEM));
	t->name = strdup(name);
	//t->name=name;
	t->type = type;
	t->action = action;
	t->arg = param;
	t->enabled = 1;
	return t;
}

void gn_menu_free_item(GN_MENU_ITEM *item)
{

    if(item != NULL){
	//printf("gn_menu_free_item(%s)\n", item->name);
	free(item->name);
	item->name = NULL;
	free(item);
	item = NULL;
    }	
}


GN_MENU_ITEM *gn_menu_get_item_by_index(GN_MENU *gmenu, int index) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;
	int i = 0;
	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (gitem->enabled) {
			if (i == index) return gitem;
			i++;
		}

	}
	return NULL;
}


int test_action(GN_MENU_ITEM *self, void *param) {
	printf("Action!!\n");
	return 0;
}

static int load_state_action(GN_MENU_ITEM *self, void *param) {
	static Uint32 slot = 0;
	SDL_Rect dstrect = {24 + 75, 16 + 66, 304 / 2, 224 / 2};
	SDL_Rect dstrect_binding = {24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4};
	//SDL_Rect dst_r={24,16,304,224};
	//SDL_Event event;
	SDL_Surface *tmps, *slot_img;
	char slot_str[32];

	Uint32 nb_slot = how_many_slot(conf.game);

	if (slot>nb_slot-1){
	    // remember the last selected slot.
	    slot = nb_slot-1;
	    
	}
	if (nb_slot == 0) {
	    gn_popup_info("Load State", "There is currently no save state available");
	    return 0; // nothing to do
	}

	//slot_img=load_state_img(conf.game,slot);
	tmps = load_state_img(conf.game, slot);
	if (tmps == NULL)
	{
	    gn_popup_info("Load State", "Failed to load state %s", slot);
	    return 0; // nothing to do
	}

	slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);

	while (1) {
		//if (back) SDL_BlitSurface(back,NULL,menu_buf,&dst_r);
		//else SDL_FillRect(menu_buf,NULL,COL32_TO_16(0x11A011));
		draw_back();
		SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
		SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);

		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, "Load State");
		sprintf(slot_str, "Slot Number %03d", slot);
		//draw_string(menu_buf,sfont,24+102,16+40,slot_str);
		draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

		if (slot > 0) draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
		if (slot < nb_slot - 1) draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		switch (wait_event()) {
			case GN_LEFT:
				if (slot > 0){
				    slot--;
				    SDL_FreeSurface(slot_img);
				    slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot), menu_buf->format, SDL_SWSURFACE);
				}
				break;
			case GN_RIGHT:
				if (slot < nb_slot - 1){
				    slot++;
				    SDL_FreeSurface(slot_img);
				    slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot), menu_buf->format, SDL_SWSURFACE);
				}
				break;
			case GN_A:
				SDL_FreeSurface(slot_img);
				return MENU_STAY;
				break;
			case GN_B:
			case GN_C:
				SDL_FreeSurface(slot_img);
				load_state(conf.game, slot);
				printf("Load state!!\n");
				return MENU_RETURNTOGAME;
				break;
			default:
				break;
		}
	}
	return 0;
}

static int save_state_action(GN_MENU_ITEM *self, void *param) {
	static Uint32 slot = 0;
	SDL_Rect dstrect = {24 + 75, 16 + 66, 304 / 2, 224 / 2};
	SDL_Rect dstrect_binding = {24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4};
	SDL_Surface *tmps, *slot_img = NULL;
	char slot_str[32];
	Uint32 nb_slot = how_many_slot(conf.game);

	if (slot > nb_slot)
		slot = nb_slot;

	if (nb_slot != 0 && slot < nb_slot) {
		tmps = load_state_img(conf.game, slot);
		slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);
	}

	while (1) {
		draw_back();
		if (slot != nb_slot) {
			SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
			SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);
		} else {
			SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
			SDL_FillRect(menu_buf, &dstrect, COL32_TO_16(0xA0B0B0));
			draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER, "Create a new Slot");
		}

		draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, "Save State");
		sprintf(slot_str, "Slot Number %03d", slot);
		draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

		if (slot > 0) draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
		if (slot < nb_slot) draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);

		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);

		switch (wait_event()) {
			case GN_LEFT:
				if (slot > 0) slot--;
				if (slot != nb_slot)
				{
				    SDL_FreeSurface(slot_img);
				    slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
						menu_buf->format, SDL_SWSURFACE);
				}
				break;
			case GN_RIGHT:
				if (slot < nb_slot) slot++;
				if (slot != nb_slot)
				{
				    SDL_FreeSurface(slot_img);
				    slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
						menu_buf->format, SDL_SWSURFACE);
				}
				break;
			case GN_A:
				SDL_FreeSurface(slot_img);
				return MENU_STAY;
				break;
			case GN_B:
			case GN_C:
				SDL_FreeSurface(slot_img);
				save_state(conf.game, slot);
				printf("Save state!!\n");
				return MENU_RETURNTOGAME;
				break;
			default:
				break;
		}
	}
	return 0;
}

static int credit_action(GN_MENU_ITEM *self, void *param) {

	return MENU_STAY;
}

static int exit_action(GN_MENU_ITEM *self, void *param) {
	//exit(0);
//	if (gn_popup_question("Quit?","Do you really want to quit gngeo?")==0)
//		return 0;
	return MENU_EXIT;
}


int menu_get_key(void)
{
    SDL_Event event;
    SDL_WaitEvent(&event);
    if(event.type == SDL_KEYDOWN)
    {
	switch (event.key.keysym.sym)
	{		    
	    case SDLK_TAB:
		return GN_MENU_KEY;	
	    case SDLK_UP:
		return GN_UP;	
	    case SDLK_DOWN:
		return GN_DOWN;	
	    case SDLK_LEFT:
		return GN_LEFT;
	    case SDLK_RIGHT:
		return GN_RIGHT;	
	    case SDLK_ESCAPE:
	    case SDLK_BACKSPACE:
		return GN_A;
	    case SDLK_RETURN:
	    case SDLK_KP_ENTER:
		return GN_B;
	    default:
		break;
	}
    }	
    return 0;
}


int menu_event_handling(struct GN_MENU *self) {
	GN_MENU_ITEM *mi;
	int a;
	LIST *l;
	int k = menu_anim ? wait_event() : menu_get_key();
	switch (k) {
		case GN_UP:
			if (self->current > 0)
				self->current--;
			else
				self->current = self->nb_elem - 1;
			break;
		case GN_DOWN:
			if (self->current < self->nb_elem - 1)
				self->current++;
			else
				self->current = 0;
			break;

		case GN_LEFT:
			self->current -= 10;
			if (self->current < 0) self->current = 0;
			break;
		case GN_RIGHT:
			self->current += 10;
			if (self->current >= self->nb_elem) self->current = self->nb_elem - 1;
			break;
		case GN_A:
			return MENU_CLOSE;
			break;
		case GN_B:
		case GN_C:
			//l = list_get_item_by_index(self->item, self->current);
			mi = gn_menu_get_item_by_index(self, self->current);
			if (mi && mi->action) {
				reset_event();   
				if ((a = mi->action(mi, NULL))>0)
/*
					if (a == MENU_CLOSE) return MENU_STAY;
					else
*/
						return a;
			}
			break;
		default:
			break;

	}
	return -1;
}

GN_MENU *create_menu(char *name, int type,
		int (*action)(struct GN_MENU *self),
		void (*draw)(struct GN_MENU *self)) {
	GN_MENU *gmenu;
	gmenu = malloc(sizeof (GN_MENU));
	gmenu->title = name;
	gmenu->nb_elem = 0;
	gmenu->current = 0;
	gmenu->draw_type = type;
	if (action)
		gmenu->event_handling = action;
	else
		gmenu->event_handling = menu_event_handling;
	if (draw)
		gmenu->draw = draw;
	else
		gmenu->draw = draw_menu;
	gmenu->item = NULL;
	return gmenu;
}

void free_menu_items(GN_MENU* menu)
{
    if(menu == NULL)
	return;
    
    LIST* item = menu->item; 
    LIST* tmp;
    int i = 0;
    //printf("free_menu_items %d items in %s\n", menu->nb_elem, menu->title);
    while((item != NULL) && (i < menu->nb_elem))
    {
	GN_MENU_ITEM *mi = (GN_MENU_ITEM *)item->data;
	if(mi != NULL)
	{
	    gn_menu_free_item(mi);
	}

	tmp = item;	
	item = item->next;
	i++;
	free(tmp);
    }
    free(menu);
    menu = NULL;

}    

GN_MENU_ITEM *gn_menu_add_item(GN_MENU *gmenu, char *name, int type,
		int (*action)(struct GN_MENU_ITEM *self, void *param), void *param) {
	GN_MENU_ITEM *gitem;
	gitem = gn_menu_create_item(name, type, action, param);
	gmenu->item = list_append(gmenu->item, (void*) gitem);
	gmenu->nb_elem++;
	return gitem;
}

GN_MENU_ITEM *gn_menu_get_item_by_name(GN_MENU *gmenu, char *name) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;

	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (strncmp(gitem->name, name, 128) == 0 && gitem->enabled != 0) {
			return gitem;
		}
	}
	return NULL;
}

void gn_menu_disable_item(GN_MENU *gmenu, char *name) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;

	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (strncmp(gitem->name, name, 128) == 0 && gitem->enabled != 0) {
			gitem->enabled = 0;
			gmenu->nb_elem--;
			return;
		}
	}
}

void gn_menu_enable_item(GN_MENU *gmenu, char *name) {
	GN_MENU_ITEM *gitem;
	LIST *l = gmenu->item;

	for (l = gmenu->item; l; l = l->next) {
		gitem = (GN_MENU_ITEM *) l->data;
		if (strcmp(gitem->name, name) == 0 && gitem->enabled == 0) {
			gitem->enabled = 1;
			gmenu->nb_elem++;
			return;
		}
	}
}

int icasesort(const struct dirent **a, const struct dirent **b) {
	const char *ca = (*a)->d_name;
	const char *cb = (*b)->d_name;
	return strcasecmp(ca, cb);
}

static int romnamesort(void *a, void *b) {
	GN_MENU_ITEM *ga = (GN_MENU_ITEM *) a;
	GN_MENU_ITEM *gb = (GN_MENU_ITEM *) b;

	return strcmp(ga->name, gb->name);
}
extern char romerror[1024];

static int loadrom_action(GN_MENU_ITEM *self, void *param) {
	char *game = (char*) self->arg;

	printf("Loading %s\n", game);
	close_game();
	if (conf.sound) close_sdl_audio();

	if (init_game(game) != true) {
		printf("Can't init %s...\n", game);
		gn_popup_error("Error! :", "Gngeo Couldn't init %s: \n\n%s\n"
				"Maybe the romset you're using is too old"
				, game,romerror);
		return MENU_STAY;
	}

	return MENU_RETURNTOGAME;
}

static int scan_romdir()
{
    DIR* d = opendir(CF_STR(cf_get_item_by_name("rompath")));
    if(d == NULL)
    {
	perror("opendir");
	return -1;
    }
    int n = 0;
    while(readdir(d) != NULL)
    {
	n++;
    }	
    closedir(d);
    return n;
}

void init_rom_browser_menu(void) {
	int i;
	char filename[strlen(CF_STR(cf_get_item_by_name("rompath"))) + 256];
	struct stat filestat;
	ROM_DEF *drv = NULL;
	//char name[32];
	static int nb_roms = 0;
	static int nb_files = 0;
	if(rbrowser_menu != NULL)
	{
	    int n = scan_romdir();
	    if(nb_files == n)
	    {
		//printf("re-scanning romdir not needed\n");
		return;
	    }
	    nb_files = n;
	    //printf("re-scanning romdir needed\n");
	    free_menu_items(rbrowser_menu);
	}
	else
	{
	    nb_files = scan_romdir();
	}

	rbrowser_menu = create_menu("Load Game", MENU_SMALL, NULL, NULL);
	i = 0;
	while (romlist[i]) {
		sprintf(filename, "%s/%s.zip", CF_STR(cf_get_item_by_name("rompath")), romlist[i]);
		if (stat(filename, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
			if ((drv = dr_check_zip(filename)) != NULL) {
				rbrowser_menu->item = list_insert_sort(rbrowser_menu->item,
						(void*) gn_menu_create_item(drv->longname, MENU_ACTION, loadrom_action, strdup(drv->name)),
						romnamesort
						);
				rbrowser_menu->nb_elem++;


				//printf("Parent=%s\n",drv->parent);
				//if (strcmp(drv->parent,"neogeo")!=0)
				//	gn_menu_disable_item(rbrowser_menu,drv->longname);

				free(drv);
				nb_roms++;
				i++;
				continue;
			}
		}

		sprintf(filename, "%s/%s.gno", CF_STR(cf_get_item_by_name("rompath")), romlist[i]);
		if (stat(filename, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
			char *gnoname = dr_gno_romname(filename);
			if (gnoname != NULL && ((drv = res_load_drv(gnoname)) != NULL) ) {
				rbrowser_menu->item = list_insert_sort(rbrowser_menu->item,
						(void*) gn_menu_create_item(drv->longname, MENU_ACTION, loadrom_action, strdup(filename)),
						romnamesort);
				rbrowser_menu->nb_elem++;
				nb_roms++;
				free(drv);
			}
			free(gnoname);
		}
		i++;
	}

	if (nb_roms == 0) {    
		rbrowser_menu->item = list_append(rbrowser_menu->item,
				(void*) gn_menu_create_item("No Games Found...", MENU_ACTION, NULL, NULL));
		rbrowser_menu->nb_elem++;
	
#ifdef SYMBIAN	
		gn_popup_info("No Games Found", "games and bios should be in %s", symbian_gngeo_romsdir());
#endif	
	}
}

#ifdef SYMBIAN
int rom_browser_menu(void) {
	static Uint32 init = 0;
	int a;
	//if (init == 0)
	{
		//init = 1;
		draw_back();
		draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Scanning ...");
		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		init_rom_browser_menu();
	}

	while (1) {
		rbrowser_menu->draw(rbrowser_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = rbrowser_menu->event_handling(rbrowser_menu)) > 0) {
			if (a == MENU_CLOSE)
				return MENU_STAY;
			else
				return a;
		}
	}
}

#else

static volatile int scaning = 0;

int rom_browser_scanning_anim(void *data) {
	int i = 0;
	while (scaning) {
		draw_back();
		if (i > 20)
			draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Scanning ...");
		else
			draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Scanning");
		SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
		screen_update();
		frame_skip(0);
		i++;
		if (i > 40) i = 0;
	}
	return 0;
}

int rom_browser_menu(void) {
	static Uint32 init = 0;
	int a;
	SDL_Thread *anim_th;

	//if (init == 0)
	{
		//init = 1;
		scaning = 1;
		anim_th = SDL_CreateThread(rom_browser_scanning_anim, NULL);
		init_rom_browser_menu();
		scaning = 0;
		SDL_WaitThread(anim_th, NULL);
		
	}

	while (1) {
		rbrowser_menu->draw(rbrowser_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = rbrowser_menu->event_handling(rbrowser_menu)) > 0)
		{
			if (a == MENU_CLOSE)
				return MENU_STAY;
			else
				return a;
		}
	}
}

#endif

static int rbrowser_action(GN_MENU_ITEM *self, void *param) {
	//exit(0);
	return rom_browser_menu();
}

static int toggle_fullscreen(GN_MENU_ITEM *self, void *param) {
	screen_fullscreen();
	self->val = 1 - self->val;
	cf_item_has_been_changed(cf_get_item_by_name("fullscreen"));
	CF_BOOL(cf_get_item_by_name("fullscreen")) = self->val;
	return MENU_STAY;
}


static GN_MENU *controls_menu[2] = {NULL, NULL};

static char* gngeo_keynames[] = {
    "NONE",
    "A",
    "B",
    "C",
    "D",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "START",
    "COIN",
    "MENU",
    "HOTKEY1",
    "HOTKEY2",
    "HOTKEY3",
    "HOTKEY4",
    "pNcontrol"
};

static int change_screenmode(GN_MENU_ITEM *self, void *param)
{

	return 0;
}

static int apply_control_changes(char* pcontrol)
{
    char* controls = CF_STR(cf_get_item_by_name(pcontrol));
    char new_controls[255];
    memset(new_controls, 0, 255);
    //printf("current controls %s len: %ld\n", controls, strlen(controls));
    int menu_idx = (strcmp(pcontrol, "p1control")==0) ? 0:1;
    GN_MENU* menu = controls_menu[menu_idx]; 
    LIST* item = menu->item;  
    int i = 0;
    int ret = 1;
    while((item != NULL) && (i < menu->nb_elem))
    {
	 
	GN_MENU_ITEM *mi = (GN_MENU_ITEM *)item->data;
	if(mi == NULL)
	{
	    return 0;
	}

	char* item_str = strdup(mi->name);
	if(item_str == NULL)
	{
	    return 0;
	}

	char* sep1 = strchr(item_str, ' ');
	if(sep1 == NULL)
	{
	    free(item_str);
	    return 0;
	}

	*sep1 = '=';
	char* sep2 = strchr(item_str, '[');
	if(sep2 == NULL)
	{
	    free(item_str);
	    return 0;
	}
	sep2--; // space
	*sep2 = '\0';

	strcat(new_controls, item_str);
	if(i < (menu->nb_elem)-1)
	{    
	    strcat(new_controls, ",");
	}  	
	item = item->next;
	i++;
	free(item_str);
    }

    memset(controls, 0, strlen(controls));
    strcpy(controls, new_controls);
    controls = CF_STR(cf_get_item_by_name(pcontrol));
    //printf("new controls %s len: %ld\n", controls, strlen(controls));
    create_joymap_from_string(1,CF_STR(cf_get_item_by_name(pcontrol)));
    return 1;
}

static void save_control_changes(char* pcontrol)
{

    cf_item_has_been_changed(cf_get_item_by_name(pcontrol));
    cf_save_file(NULL, 0);
}

static int setup_ctrlkey_action(GN_MENU_ITEM *self, void *param)
{
    SDL_Event event;
    int controls_changed = 0;
    char* keyname = (char*)self->arg;
    char* pcontrol = gngeo_keynames[16];   
    char* item_str = strdup(self->name); // save the original item string
    //printf("keyname: %s\n", keyname);
    //printf("pcontrol: %s\n", pcontrol); 
    int menu_idx = (strcmp(pcontrol, "p1control")==0) ? 0:1;
    GN_MENU* menu = controls_menu[menu_idx];    
    menu->title = "press ENTER to save";
    while (SDL_WaitEvent(&event))
    {
	if(event.type == SDL_KEYDOWN)
	{
	    int keycode = event.key.keysym.sym;
	    switch (keycode)
	    {

		case SDLK_RETURN:
		    free(item_str);
		    // save controls 
		    if(controls_changed){
			apply_control_changes(pcontrol);
			return 2;
		    }		
		    return MENU_STAY;	
		case SDLK_ESCAPE:
		case SDLK_BACKSPACE:
		    //reset original item string
		    sprintf(self->name, "%s", item_str);
		    free(item_str);
		    return MENU_STAY;
		case SDLK_HOME:
		    // ignore CALL/GREEN Key, used for taking screenshots
		    break;
		default:
		    controls_changed = 1;
		    sprintf(self->name, "%s K%d [%s]", keyname, keycode, SDL_GetKeyName(keycode));
		    menu->draw(menu);
		    break;
	    }
	}	    
    }

    free(item_str);
    return MENU_STAY;
}

static void create_controls_menu(int p_num)
{
    int i = 0;
    char control[64] = {0};
    char* menu_title = "press ENTER to edit";
    char* pcontrol = (p_num==1) ?  "p1control" : "p2control";
    int menu_idx = p_num-1;
    if(controls_menu[menu_idx] == NULL)
    {
	controls_menu[menu_idx] = create_menu(menu_title, MENU_SMALL, NULL, NULL);
    }

    GN_MENU* menu = controls_menu[menu_idx];
    while(i < SDLK_LAST)
    {
	int p = jmap->key[i].player;
	int k = jmap->key[i].map;
	i++; 
	if(p != p_num || k == 0)
	{
	    continue;
	}
   
	//sprintf(control, "%s %s", keynames[k], SDL_GetKeyName(i-1)); // why i-1 ?
	sprintf(control, "%s K%d [%s]", gngeo_keynames[k], i-1, SDL_GetKeyName(i-1));
	menu->item = list_append(menu->item,(void*) gn_menu_create_item(control, MENU_ACTION, setup_ctrlkey_action, (void*)gngeo_keynames[k]));
	menu->nb_elem++;
    }
}


static int toggle_menu_anim(GN_MENU_ITEM *self, void *param)
{
    self->val = !self->val;
    menu_anim = self->val;
    CONF_ITEM * cf_item = cf_get_item_by_name("menu_anim");
    CF_BOOL(cf_item) = menu_anim;
    cf_item_has_been_changed(cf_item);   
    return MENU_STAY;
}

#ifdef SYMBIAN
static int toggle_landscapemode(GN_MENU_ITEM *self, void *param)
{
    int ret = symbian_ui_orientation_setup();
    gn_popup_info("config changed", "Please restart the emulator.");
    self->val = ret;//self->val;   
    return MENU_STAY;
}

static int change_audio_volume(GN_MENU_ITEM *self, void *param)
{
    SDL_Event event;
    int current_vol = symbian_audio_volume_get();
    int old_vol = current_vol;
    char* title = "Change Audio Volume";
    char msg[100];
    sprintf(msg, "use UP/DOWN");
    reset_event();
    draw_back();
    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, title);
    draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER, msg);
    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_update();

    while (SDL_WaitEvent(&event))
    {
	if(event.type == SDL_KEYDOWN)
	{
	    int keycode = event.key.keysym.sym;
	    switch (keycode)
	    {
		case SDLK_UP:
		    current_vol = SDL_min(current_vol+10, 256);
		    symbian_audio_volume_set(current_vol, 0);
		    break;
		case SDLK_DOWN:
		    current_vol = SDL_max(current_vol-10, 0);
		    symbian_audio_volume_set(current_vol, 0);
		    break;
		case SDLK_ESCAPE:
		case SDLK_RETURN:
		    if(current_vol != old_vol)
		    {
			CONF_ITEM * cf_item = cf_get_item_by_name("audio_volume");
			CF_VAL(cf_item) = current_vol;
			cf_item_has_been_changed(cf_item);
		    }	
		    return MENU_STAY;
		default:
		    break;
	    }

	    // redraw
	    sprintf(msg, "current volume %d", current_vol);
	    sprintf(self->str, "%d", current_vol);
	    draw_back();
	    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, title);
	    draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER, msg);
	    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	    screen_update();
	}	    
    }
    
    return MENU_STAY;
}    
#endif

static int setup_controls(GN_MENU_ITEM *self, void *param)
{
 	int i = 0;   
	int controls_changed = 0;
	int p_num = (strcmp(self->str, "P2")==0) ? 2 :1;
	char* pcontrol = (p_num==1) ?  "p1control" : "p2control";
	gngeo_keynames[16] = pcontrol; // set current player controls id
	//printf("pcontrol: %s\n", pcontrol);
	int menu_idx = p_num - 1;
	GN_MENU* menu = controls_menu[menu_idx]; 
	while(1)
	{
	    menu->title = "press ENTER to edit";
	    menu->draw(menu);
	    switch(menu->event_handling(menu))
	    {
		case 1: // SDLK_ESCAPE
		    if(controls_changed && gn_popup_question("Save Changes", "save to the config file?") == 2)
		    {
			save_control_changes(pcontrol);
		    }
		    return MENU_STAY;
		case 2:
		    //printf("controls_changed\n");
		    controls_changed = 1;    
		    break;
	    }
	}
	return MENU_STAY;
}

static int toggle_wide(GN_MENU_ITEM *self, void *param) {
	self->val = 1 - self->val;

	cf_item_has_been_changed(cf_get_item_by_name("wide"));
	CF_BOOL(cf_get_item_by_name("wide")) = self->val;
	screen_reinit();
	return MENU_STAY;
}

#ifndef SYMBIAN
static int toggle_vsync(GN_MENU_ITEM *self, void *param) {

	self->val = 1 - self->val;
	conf.vsync = self->val;
	cf_item_has_been_changed(cf_get_item_by_name("vsync"));
	CF_BOOL(cf_get_item_by_name("vsync")) = self->val;
	screen_reinit();
	return MENU_STAY;
}
#endif

static int toggle_autoframeskip(GN_MENU_ITEM *self, void *param) {
	self->val = 1 - self->val;
	conf.autoframeskip = self->val;
	cf_item_has_been_changed(cf_get_item_by_name("autoframeskip"));
	CF_BOOL(cf_get_item_by_name("autoframeskip")) = self->val;
	reset_frame_skip();
	return MENU_STAY;
}

static int toggle_sleepidle(GN_MENU_ITEM *self, void *param) {
	self->val = 1 - self->val;
	conf.sleep_idle = self->val;
	cf_item_has_been_changed(cf_get_item_by_name("sleepidle"));
	CF_BOOL(cf_get_item_by_name("sleepidle")) = self->val;

	return MENU_STAY;
}

static int toggle_showfps(GN_MENU_ITEM *self, void *param) {
	self->val = 1 - self->val;
	conf.show_fps = self->val;
	cf_item_has_been_changed(cf_get_item_by_name("showfps"));
	CF_BOOL(cf_get_item_by_name("showfps")) = self->val;

	return MENU_STAY;
}

static int change_effect_action(GN_MENU_ITEM *self, void *param) {
	char *ename = (char *) self->arg;
	printf("Toggle to effect %s\n", self->name);
	if (strcmp(ename, "none") != 0 || strcmp(ename, "soft") != 0) {
		scale = 1;
	}
	strncpy(CF_STR(cf_get_item_by_name("effect")), ename, 254);
	cf_item_has_been_changed(cf_get_item_by_name("effect"));
#ifdef SYMBIAN
	cf_save_file(NULL, 0);
	gn_popup_info("config changed", "Please restart the emulator.");
#else	
	screen_reinit();
#endif	
	return MENU_STAY;
}
extern effect_func effect[];

static int change_effect(GN_MENU_ITEM *self, void *param) {
	static int init = 0;
	int a;
	int i;
	if (init == 0) {

		init = 1;
		effect_menu = create_menu("Choose an Effect", MENU_SMALL, NULL, NULL);

		i = 0;
		while (effect[i].name != NULL) {
			effect_menu->item = list_append(effect_menu->item,
					(void*) gn_menu_create_item(effect[i].desc, MENU_ACTION, change_effect_action, (void*) effect[i].name));
			effect_menu->nb_elem++;
			i++;
		}
	}
	while (1) {
		effect_menu->draw(effect_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
if ((a = effect_menu->event_handling(effect_menu)) > 0) {
			self->str = CF_STR(cf_get_item_by_name("effect"));
			return MENU_STAY;
		}
	}
	return 0;
}

extern int neo_sound_initialized;
static int change_samplerate_action(GN_MENU_ITEM *self, void *param) {
	int rate = (int) self->arg;

	if (rate != 0) {

		CF_VAL(cf_get_item_by_name("samplerate")) = rate;
		cf_item_has_been_changed(cf_get_item_by_name("samplerate"));
		if (conf.sound && conf.game)
			close_sdl_audio();
		else
			cf_item_has_been_changed(cf_get_item_by_name("sound"));
		conf.sound = 1;
		CF_BOOL(cf_get_item_by_name("sound")) = 1;
		conf.sample_rate = rate;
		//init_sdl_audio();
		//YM2610ChangeSamplerate(conf.sample_rate);
		if(!neo_sound_initialized)
		{
		    gn_popup_info("config changed", "Please reload the game.");
		    return MENU_CLOSE;
		}

		if (conf.game) {
			init_sdl_audio();
			YM2610ChangeSamplerate(conf.sample_rate);
		}


	} else {
		if (conf.sound)
		    cf_item_has_been_changed(cf_get_item_by_name("sound"));
		conf.sound = 0;
		conf.sample_rate = 0;
		if (conf.game) close_sdl_audio();
		CF_BOOL(cf_get_item_by_name("sound")) = 0;
	}

	return MENU_CLOSE;
}

static int change_samplerate(GN_MENU_ITEM *self, void *param) {
	static int init = 0;
	int a;
	GN_MENU_ITEM *gitem;
	if (init == 0) {
		init = 1;
		srate_menu = create_menu("Choose a sample rate", MENU_SMALL, NULL, NULL);
		gitem = gn_menu_create_item("No sound", MENU_ACTION,
				change_samplerate_action, (void*) 0);
		srate_menu->item = list_append(srate_menu->item, (void*) gitem);
		srate_menu->nb_elem++;

		gitem = gn_menu_create_item("11025 (Fast but poor quality)", MENU_ACTION,
				change_samplerate_action, (void*) 11025);
		srate_menu->item = list_append(srate_menu->item, (void*) gitem);
		srate_menu->nb_elem++;

		gitem = gn_menu_create_item("22050 (Good compromise)", MENU_ACTION,
				change_samplerate_action, (void*) 22050);
		srate_menu->item = list_append(srate_menu->item, (void*) gitem);
		srate_menu->nb_elem++;

		gitem = gn_menu_create_item("44100 (Best quality)", MENU_ACTION,
				change_samplerate_action, (void*) 44100);
		srate_menu->item = list_append(srate_menu->item, (void*) gitem);
		srate_menu->nb_elem++;
	}

	//	gn_menu_disable_item(srate_menu,"No sound");

	while (1) {
		srate_menu->draw(srate_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = srate_menu->event_handling(srate_menu)) > 0) {
			if (conf.sound)
				sprintf(self->str, "%d", conf.sample_rate);
			else
				sprintf(self->str, "No sound");

			return MENU_STAY;
		}
	}
	return 0;
}


static int change_cpu_clock(GN_MENU_ITEM *self, void *param)
{
    SDL_Event event;
    int old_val = (int)self->arg;
    int current_val = old_val;
    int is_z80 = strstr(self->name, "z80") != NULL;
    char* title = is_z80 ? "Overclock the Z80": "Overclock the 68k";
    char msg[100];
    sprintf(msg, "use UP/DOWN to change the value");
    reset_event();
    draw_back();
    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, title);
    draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER, msg);
    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_update();

    while (SDL_WaitEvent(&event))
    {
	if(event.type == SDL_KEYDOWN)
	{
	    int keycode = event.key.keysym.sym;
	    switch (keycode)
	    {
		case SDLK_UP:
		    current_val = SDL_min(100, current_val+1);
		    break;
		case SDLK_DOWN:
		    current_val = SDL_max(0, current_val-1);
		    break;
		case SDLK_ESCAPE:
		case SDLK_BACKSPACE:
		    sprintf(self->str, "%d", old_val);
		    return MENU_STAY;
		case SDLK_RETURN:
		    if(current_val != old_val)
		    {
			char* item_name = is_z80 ? "z80clock" : "68kclock";
			CONF_ITEM * cf_item = cf_get_item_by_name(item_name);
			CF_VAL(cf_item) = current_val;
			cf_item_has_been_changed(cf_item);
			cf_save_file(NULL, 0);
			gn_popup_info("config changed", "You must restart the emulator to apply this change.");
		    }	
		    return MENU_STAY;
		default:
		    break;
	    }

	    // redraw
	    sprintf(msg, "current value %d, use ENTER to save the value.", current_val);
	    sprintf(self->str, "%d", current_val);
	    draw_back();
	    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, title);
	    draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER, msg);
	    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
	    screen_update();
	}	    
    }
    
    return MENU_STAY;
} 

static int save_conf_action(GN_MENU_ITEM *self, void *param) {
	int type = (int) self->arg;
	if (type == 0)
		cf_save_file(NULL, 0);
	else {
		char *gpath;
		char *drconf;
		char *name = memory.rom.info.name;
#ifdef EMBEDDED_FS
		gpath = ROOTPATH"conf/";
#else
		gpath = get_gngeo_dir();
#endif
		size_t len = strlen(gpath) + strlen(name) + strlen(".cf") + 1;
		
#ifdef USE_ALLOCA 
		drconf = alloca(len);
#else 
		drconf = (char *)malloc(len);
		CHECK_ALLOC(drconf);
		memset(drconf, 0, len); 
#endif 
		sprintf(drconf, "%s%s.cf", gpath, name);
		cf_save_file(drconf, 0);
#ifndef USE_ALLOCA 
		free(drconf);
#endif
	}
	return 1;
}

#define RESET_BOOL(name,id) gitem=gn_menu_get_item_by_name(option_menu,name);\
if (gitem) gitem->val = CF_BOOL(cf_get_item_by_name(id));

static void reset_menu_option(void) {
	GN_MENU_ITEM *gitem;
	//gitem=gn_menu_get_item_by_name(option_menu,"Fullscreen");
	//if (gitem) gitem->val = CF_BOOL(cf_get_item_by_name("fullscreen"));
	RESET_BOOL("Fullscreen","fullscreen");
	RESET_BOOL("Vsync","vsync");
	RESET_BOOL("Auto Frame Skip","autoframeskip");
	RESET_BOOL("Sleep while idle","sleepidle");
	RESET_BOOL("Show FPS","showfps");
#ifdef PANDORA
	RESET_BOOL("16/9","wide");
#endif
// FIX: check for null	
	gitem=gn_menu_get_item_by_name(option_menu,"Effect");
	if(gitem != NULL){
	    gitem->str = CF_STR(cf_get_item_by_name("effect"));
	}
	gitem=gn_menu_get_item_by_name(option_menu,"Sample Rate");
	if (gitem != NULL)
	{
	    if (conf.sound)
		sprintf(gitem->str, "%d", conf.sample_rate);
	    else
		sprintf(gitem->str, "No sound");
	}    
}

static int option_action(GN_MENU_ITEM *self, void *param) {
	//exit(0);
	int a;
	reset_menu_option();
	while (1) {
		option_menu->draw(option_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = option_menu->event_handling(option_menu)) > 0) {
			reset_menu_option();
			return MENU_STAY;
		}
	}
}


static char srate[32];
void gn_init_menu(void) {
	GN_MENU_ITEM *gitem;
	main_menu = create_menu(NULL, MENU_BIG, NULL, NULL);

	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Load game", MENU_ACTION, rbrowser_action, NULL));
	main_menu->nb_elem++;


	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Load state", MENU_ACTION, load_state_action, NULL));
	main_menu->nb_elem++;
	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Save state", MENU_ACTION, save_state_action, NULL));
	main_menu->nb_elem++;

	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Option", MENU_ACTION, option_action, NULL));
	main_menu->nb_elem++;

/*
	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Credit", MENU_ACTION, credit_action, NULL));
	main_menu->nb_elem++;
*/

	main_menu->item = list_append(main_menu->item,
			(void*) gn_menu_create_item("Exit", MENU_ACTION, exit_action, NULL));
	main_menu->nb_elem++;

	option_menu = create_menu("Options", MENU_SMALL, NULL, NULL);

#ifdef SYMBIAN

	gitem = gn_menu_create_item("Landscape Mode", MENU_CHECK, toggle_landscapemode, NULL);
	gitem->val = symbian_ui_orientation_get();
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

#endif	
	gitem = gn_menu_create_item("Menu Animation", MENU_CHECK, toggle_menu_anim, NULL);
	menu_anim = CF_BOOL(cf_get_item_by_name("menu_anim"));
	gitem->val = menu_anim;
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	gitem = gn_menu_create_item("Controls", MENU_LIST, setup_controls, NULL);
	gitem->str = "P1";
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;
	create_controls_menu(1); // P1
	/*
	gitem = gn_menu_create_item("Controls", MENU_LIST, setup_controls, NULL);
	gitem->str = "P2";
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;
    	create_controls_menu(2); // P2
	*/ 


#ifndef SYMBIAN	
	gitem = gn_menu_create_item("Fullscreen", MENU_CHECK, toggle_fullscreen, NULL);
	gitem->val = CF_BOOL(cf_get_item_by_name("fullscreen"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;
#endif

#ifdef PANDORA
	gitem = gn_menu_create_item("16/9", MENU_CHECK, toggle_wide, NULL);
	gitem->val = CF_BOOL(cf_get_item_by_name("wide"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;
#endif
#ifndef SYMBIAN 
	// TODO: find why SDL_DOUBLEBUF crashes on Symbian.
	gitem = gn_menu_create_item("Vsync", MENU_CHECK, toggle_vsync, NULL);
	gitem->val = CF_BOOL(cf_get_item_by_name("vsync"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;
#endif
	gitem = gn_menu_create_item("Auto Frame Skip", MENU_CHECK, toggle_autoframeskip, NULL);
	gitem->val = CF_BOOL(cf_get_item_by_name("autoframeskip"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	gitem = gn_menu_create_item("Sleep while idle", MENU_CHECK, toggle_sleepidle, NULL);
	gitem->val = CF_BOOL(cf_get_item_by_name("sleepidle"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	gitem = gn_menu_create_item("Show FPS", MENU_CHECK, toggle_showfps, NULL);
	gitem->val = CF_BOOL(cf_get_item_by_name("showfps"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	gitem = gn_menu_create_item("Effect", MENU_LIST, change_effect, NULL);
	gitem->str = CF_STR(cf_get_item_by_name("effect"));
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;
	
#ifdef SYMBIAN
	gitem = gn_menu_create_item("Audio Volume", MENU_LIST, change_audio_volume, NULL);
	int saved_vol = CF_VAL(cf_get_item_by_name("audio_volume"));
	symbian_audio_volume_set(saved_vol, 0);
	sprintf(audio_vol, "%d", saved_vol);
	gitem->str = audio_vol;
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

#endif
	gitem = gn_menu_create_item("Sample Rate", MENU_LIST, change_samplerate, NULL);
	gitem->str = srate;
	//gitem->str = malloc(32);
	if (conf.sound)
		sprintf(gitem->str, "%d", conf.sample_rate);
	else
		sprintf(gitem->str, "No sound");
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	int z80clock_value = CF_VAL(cf_get_item_by_name("z80clock"));
	gitem = gn_menu_create_item("z80 Clock", MENU_LIST, change_cpu_clock, (void*)z80clock_value);
	sprintf(z80_clock, "%d", z80clock_value);
	gitem->str = z80_clock;
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	int m68kclock_value = CF_VAL(cf_get_item_by_name("68kclock"));
	gitem = gn_menu_create_item("68k Clock", MENU_LIST, change_cpu_clock, (void*)m68kclock_value);
	sprintf(m68k_clock, "%d", m68kclock_value);
	gitem->str = m68k_clock;
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	gitem = gn_menu_create_item("Save conf for every game", MENU_ACTION, save_conf_action, (void*) 0);
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	gitem = gn_menu_create_item("Save conf for this game", MENU_ACTION, save_conf_action, (void*) 1);
	option_menu->item = list_append(option_menu->item, (void*) gitem);
	option_menu->nb_elem++;

	yesno_menu = create_menu(NULL, MENU_SMALL, NULL, NULL);
	gitem = gn_menu_create_item("Yes", MENU_ACTION, yes_action, NULL);
	yesno_menu->item = list_append(yesno_menu->item, (void*) gitem);
	yesno_menu->nb_elem++;
	gitem = gn_menu_create_item("No", MENU_ACTION, no_action, NULL);
	yesno_menu->item = list_append(yesno_menu->item, (void*) gitem);
	yesno_menu->nb_elem++;
}


Uint32 run_menu(void) {
	static Uint32 init = 0;
	int a;

	if (init == 0) {
		init = 1;
		gn_init_menu();
	}

#ifdef SYMBIAN
	// FIX: mute audio when game is paused.
	if(conf.sound)
	{
	    symbian_audio_mute();
	}    
#endif	
	init_back();

	reset_event();
	//	conf.autoframeskip = 1;
	reset_frame_skip();

	gn_menu_disable_item(main_menu, "Load state");
	if (conf.game == NULL) {
		gn_menu_disable_item(main_menu, "Save state");
		gn_menu_disable_item(option_menu, "Save conf for this game");
	} else {
		Uint32 nb_slot = how_many_slot(conf.game);
		gn_menu_enable_item(main_menu, "Save state");
		gn_menu_enable_item(option_menu, "Save conf for this game");
		if (nb_slot > 0)
			gn_menu_enable_item(main_menu, "Load state");
	}

	while (1) {
		main_menu->draw(main_menu); //frame_skip(0);printf("fps: %s\n",fps_str);
		if ((a = main_menu->event_handling(main_menu)) > 0)
		{
#ifdef SYMBIAN
		    // reset audio volume.
		    if(conf.sound)
		    {
			symbian_audio_volume_set(symbian_audio_volume_get(), 0);
		    }    
#endif		    
		    //reset_event();
		    return a;
		}
	}
	//reset_event();
	if (conf.game == NULL) return 2; /* Exit */
	
	return 0;
}

 
void cleanup_menu()
{
    int i;
    SDL_FreeSurface(pbar_logo);   
    SDL_FreeSurface(menu_buf);
    SDL_FreeSurface(menu_back);
    // free loaded images
    for(i=0; i < 7; i++)
    {
	if(img_res[i]->data != NULL)
	{	    
	    SDL_FreeSurface(img_res[i]->img);
	    res_free_data(img_res[i]);
	}    
    }
    // free loaded fonts
    SDL_FreeSurface(mfont->bmp);
    SDL_FreeSurface(sfont->bmp);
    free(sfont);
    free(mfont);
    res_free_data(font_res[0]);
    res_free_data(font_res[1]);
    gn_menu_enable_item(main_menu, "Load state");
    gn_menu_enable_item(main_menu, "Save state");  
    free_menu_items(main_menu);    
    gn_menu_enable_item(option_menu, "Save conf for this game");    
    free_menu_items(option_menu);
    free_menu_items(rbrowser_menu);
    free_menu_items(effect_menu);
    free_menu_items(srate_menu);
    free_menu_items(yesno_menu);
    free_menu_items(controls_menu[0]); // P1
    neogeo_cleanup_state();
}

