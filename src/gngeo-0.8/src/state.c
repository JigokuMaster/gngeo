#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "SDL.h"
#include "SDL_endian.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#if defined(HAVE_LIBZ) && defined (HAVE_MMAP)
#include <zlib.h>
#endif
#ifdef SYMBIAN
#include <zlib.h>
#endif


#include "memory.h"
#include "state.h"
#include "fileio.h"
#include "screen.h"
#include "sound.h"
#include "emu.h"
//#include "streams.h"

#ifdef USE_STARSCREAM
static int m68k_flag=0x1;
#elif USE_GENERATOR68K
static int m68k_flag=0x2;
#elif USE_CYCLONE
static int m68k_flag=0x3;
#endif

#ifdef USE_RAZE
static int z80_flag=0x4;
#elif USE_MAMEZ80
static int z80_flag=0x8;
#elif USE_DRZ80
static int z80_flag=0xC;
#endif

#ifdef WORDS_BIGENDIAN
static int endian_flag=0x10;
#else
static int endian_flag=0x0;
#endif

#if defined (WII)
#define ROOTPATH "sd:/apps/gngeo/"
#elif defined (__AMIGA__)
#define ROOTPATH "/PROGDIR/data/"
#else
#define ROOTPATH ""
#endif

/*
#if !defined(HAVE_LIBZ) || !defined (HAVE_MMAP)
#define gzopen fopen
#define gzread(f,data,size) fread(data,size,1,f)
#define gzwrite(f,data,size) fwrite(data,size,1,f)
#define gzclose fclose
#define gzFile FILE
#define gzeof feof
#define gzseek fseek
#endif
*/

SDL_Surface *state_img_tmp = NULL;

void cpu_68k_mkstate(gzFile gzf,int mode);
void cpu_z80_mkstate(gzFile gzf,int mode);
void ym2610_mkstate(gzFile gzf,int mode);

Uint32 how_many_slot(char *game) {
	char *st_name;
	FILE *f;
//    char *st_name_len;
#ifdef EMBEDDED_FS
	char *gngeo_dir=ROOTPATH"save/";
#else
	char *gngeo_dir=get_gngeo_dir();
#endif
	Uint32 slot=0;
	size_t len = strlen(gngeo_dir)+strlen(game)+5;
#ifdef USE_ALLOCA	
	st_name = (char*)alloca(len);
#else
	st_name = (char*)malloc(len);
	CHECK_ALLOC(st_name);
#endif	
	while (1)
	{
#ifndef USE_ALLOCA
		memset(st_name, 0, len);
#endif    
	    	sprintf(st_name,"%s%s.%03d",gngeo_dir,game,slot);

		if (st_name && (f=fopen(st_name,"rb")))
		{
			fclose(f);
			slot++;
		} 
		else{
		    break;
		}
	}

#ifndef USE_ALLOCA
	free(st_name);
	return slot;
#endif 
}

static gzFile open_state(char *game,int slot,int mode)
{
    char *st_name;
//    char *st_name_len;
#ifdef EMBEDDED_FS
    char *gngeo_dir=ROOTPATH"save/";
#else
    char *gngeo_dir=get_gngeo_dir();
#endif
    char string[20];
    char *m=(mode==STWRITE?"wb":"rb");
    gzFile gzf;
    int  flags;
    Uint32 rate;
    size_t len = strlen(gngeo_dir)+strlen(game)+5;
#ifdef USE_ALLOCA	   
    st_name = (char*)alloca(len);
#else 
    st_name = (char*)malloc(len);
    memset(st_name, 0, len);
    CHECK_ALLOC(st_name);
#endif    
    sprintf(st_name,"%s%s.%03d",gngeo_dir,game,slot);

    if ((gzf = gzopen(st_name, m)) == NULL) {
		fprintf(stderr, "%s not found\n", st_name);
		return NULL;
    }

#ifndef USE_ALLOCA	   
    free(st_name);
#endif
    if(mode==STREAD) {

		memset(string, 0, 20);
		gzread(gzf, string, 6);
		if (strcmp(string, "GNGST3")!=0 && strcmp(string, "GNGST2")!=0) {
			fprintf(stderr, "%s is not a valid gngeo st file\n", st_name);
			gzclose(gzf);
			return NULL;
		}
		if (strcmp(string, "GNGST2")==0) state_version=ST_VER2;
		if (strcmp(string, "GNGST3")==0) state_version=ST_VER3;	
		gzread(gzf, &flags, sizeof (int));

		if (flags != (m68k_flag | z80_flag | endian_flag)) {
			fprintf(stderr, "This save state comme from a different endian architecture.\n"
					"This is not currently supported :(\n");
			gzclose(gzf);
			return NULL;
		}
	} else {
		int flags=m68k_flag | z80_flag | endian_flag;
		gzwrite(gzf, "GNGST3", 6);
		gzwrite(gzf, &flags, sizeof(int));
	}
	return gzf;
}

int mkstate_data(gzFile gzf,void *data,int size,int mode) {
	if (mode==STREAD)
		return gzread(gzf,data,size);
	return gzwrite(gzf,data,size);
}

SDL_Surface *load_state_img(char *game,int slot) {
	gzFile gzf;

	if ((gzf = open_state(game, slot, STREAD)) == NULL)
		return NULL;

	gzread(gzf, state_img_tmp->pixels, 304 * 224 * 2);


    gzclose(gzf);
    return state_img_tmp;
}


static void neogeo_mkstate(gzFile gzf,int mode) {
	GAME_ROMS r;
	memcpy(&r,&memory.rom,sizeof(GAME_ROMS));
	mkstate_data(gzf, &memory, sizeof (memory), mode);

	/* Roms info are needed (at least) for z80 bankswitch, so we need to restore
	 * it asap */
	if (mode==STREAD) memcpy(&memory.rom,&r,sizeof(GAME_ROMS));


	mkstate_data(gzf, &bankaddress, sizeof (Uint32), mode);
	mkstate_data(gzf, &sram_lock, sizeof (Uint8), mode);
	cpu_68k_mkstate(gzf, mode);
#ifndef ENABLE_940T
	mkstate_data(gzf, z80_bank,sizeof(Uint16)*4, mode);
	cpu_z80_mkstate(gzf, mode);
	ym2610_mkstate(gzf, mode);
#else
/* TODO */
#endif
}

bool save_state(char *game,int slot) {
	gzFile gzf;

	if ((gzf = open_state(game, slot, STWRITE)) == NULL)
		return false;

	gzwrite(gzf, state_img->pixels, 304 * 224 * 2);

	neogeo_mkstate(gzf,STWRITE);

	gzclose(gzf);
	return true;
}
bool load_state(char *game,int slot) {
	gzFile gzf;
	/* Save pointers */
	Uint8 *ng_lo = memory.ng_lo;
	Uint8 *fix_game_usage=memory.fix_game_usage;
	Uint8 *bksw_unscramble = memory.bksw_unscramble;
	int *bksw_offset=memory.bksw_offset;
//	GAME_ROMS r;
//	memcpy(&r,&memory.rom,sizeof(GAME_ROMS));
	
	if ((gzf = open_state(game, slot, STREAD))==NULL)
		return false;

	if (state_version==ST_VER2)
		gn_popup_error("Warning!","You're trying to load an older gngeo save state\nIt may work or not, nobody knows! ;)");

	gzread(gzf,state_img_tmp->pixels,304*224*2);

	neogeo_mkstate(gzf,STREAD);

	/* Restore them */
	memory.ng_lo=ng_lo;
	memory.fix_game_usage=fix_game_usage;
	memory.bksw_unscramble=bksw_unscramble;
	memory.bksw_offset=bksw_offset;
//	memcpy(&memory.rom,&r,sizeof(GAME_ROMS));

	cpu_68k_bankswitch(bankaddress);

	if (memory.current_vector==0)
		memcpy(memory.rom.cpu_m68k.p, memory.rom.bios_m68k.p, 0x80);
	else
		memcpy(memory.rom.cpu_m68k.p, memory.game_vector, 0x80);

	if (memory.vid.currentpal) {
		current_pal = memory.vid.pal_neo[1];
		current_pc_pal = (Uint32 *) memory.vid.pal_host[1];
	} else {
		current_pal = memory.vid.pal_neo[0];
		current_pc_pal = (Uint32 *) memory.vid.pal_host[0];
	}

	if (memory.vid.currentfix) {
		current_fix = memory.rom.game_sfix.p;
		fix_usage = memory.fix_game_usage;
	} else {
		current_fix = memory.rom.bios_sfix.p;
		fix_usage = memory.fix_board_usage;
	}

	gzclose(gzf);
	return true;
}


void neogeo_init_save_state(void)
{
    if (!state_img)
    {	state_img=SDL_CreateRGBSurface(SDL_SWSURFACE,304, 224, 16, 0xF800, 0x7E0, 0x1F, 0);
	if (!state_img_tmp){
		state_img_tmp=SDL_CreateRGBSurface(SDL_SWSURFACE,304, 224, 16, 0xF800, 0x7E0, 0x1F, 0);
	}
    }
}

void neogeo_cleanup_state()
{
    SDL_FreeSurface(state_img);
    SDL_FreeSurface(state_img_tmp);
}







