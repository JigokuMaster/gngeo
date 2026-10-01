#ifndef SYMBIAN_S60_H
#define SYMBIAN_S60_H
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

extern	void EPOC_SetAudioVolume(int v);
extern	int EPOC_GetAudioVolume();
extern	int EPOC_GetAudioMaxVolume();

void	symbian_init();
char*	symbian_gngeo_dir();
char*	symbian_gngeo_romsdir();
char*	symbian_gngeo_biosdir();
char*	symbian_gngeo_datafile();
char*	symbian_get_state_dir(char* game, int slot);
char* 	symbian_get_nvram_dir(char* game);
void	symbian_audio_volume_set(int v, int update);
int	symbian_audio_volume_get();
void	symbian_audio_mute();
bool	symbian_get_screenorientation();
bool	symbian_setup_screenorientation();

// scaller/effect functions


SDL_bool symbian_effect_init();
void     symbian_effect_scanline_update();
void     symbian_effect_scaler_update();

/*
 * High-Performance Software Stretch Routines for RGB565.
 * Replaces SDL_SoftStretch with optimized 16.16 fixed-point math.
 * 
 * IMPORTANT: 'srcStride' and 'dstStride' MUST be passed in PIXELS (16-bit words),
 * NOT BYTES! Use (surface->pitch / 2) or (surface->pitch / sizeof(uint16_t)).
 */


static inline void FastStretchRectRGB565(const unsigned short* src, int srcStride,
                                         int srcX, int srcY, int srcW, int srcH,
                                         unsigned short* dst, int dstStride,
                                         int dstX, int dstY, int dstW, int dstH,
                                         int enableScanlines) {
    int x, y;
    unsigned long xStep, xAcc, yStep, yAcc;
    int xLut[640]; /* Pre-calculated X offsets (max target width 640px) */

    /* Safety checks */
    if (!src || !dst || srcW <= 0 || srcH <= 0 || dstW <= 0 || dstH <= 0) {
        return;
    }

    /* Clamp destination width to stack LUT bounds */
    if (dstW > 640) {
        dstW = 640;
    }

    /* 1. Pre-calculate X lookup table relative to srcX offset */
    xStep = ((unsigned long)srcW << 16) / (unsigned long)dstW;
    xAcc = 0;
    for (x = 0; x < dstW; ++x) {
        xLut[x] = srcX + (int)(xAcc >> 16);
        xAcc += xStep;
    }

    /* 2. Setup Y fixed-point accumulator */
    yStep = ((unsigned long)srcH << 16) / (unsigned long)dstH;
    yAcc = 0;

    /* 3. Render loop */
    for (y = 0; y < dstH; ++y) {
        int currentSrcY = srcY + (int)(yAcc >> 16);
        const unsigned short* srcRow = src + (currentSrcY * srcStride);
        
        /* Offset destination pointer to (dstX, dstY + y) */
        unsigned short* dstRow = dst + ((dstY + y) * dstStride) + dstX;

        yAcc += yStep;

        /* Fused 50% CRT scanline pass */
        if (enableScanlines && (y & 1)) {
            for (x = 0; x < dstW; ++x) {
                unsigned short color = srcRow[xLut[x]];
                /* Shift RGB565 channels by 1 bit while masking color bleed */
                dstRow[x] = (unsigned short)((color >> 1) & 0x7BEFU);
            }
        } else {
            /* Standard scaled row pass */
            for (x = 0; x < dstW; ++x) {
                dstRow[x] = srcRow[xLut[x]];
            }
        }
    }
}


#ifdef __cplusplus
}
#endif // __cplusplus



#endif
