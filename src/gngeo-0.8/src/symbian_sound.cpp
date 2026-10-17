#include "DirectAudioStream.h"

//#define BUFFER_LEN 16384

extern "C" {
#include "sound.h"
#include "conf.h" // for CF_VAL
#include "emu.h" // for conf struct
#include "ym2610/ym2610.h"

//Uint16 play_buffer[BUFFER_LEN];

extern void YM2610Update_stream_to(int length, s16 *dst);

//extern Uint32 get_ticks(void);
#ifndef uclock_t
#define uclock_t Uint32
#endif

extern uclock_t get_ticks(void);
}

//#define NEOGEO_MVS_FP 59.1856f

static CSymbianAudioStream* audioStreamer = NULL;


#define SLICE 64
static long long acc; 
static uclock_t last;
static int owed;
static s16 abuf[2 * 2048];

/* once per frame, after video */
void SymbianAudioStream_Update()
{

    if (!audioStreamer) return;

    uclock_t now = get_ticks();

    /*static int started = 0;
    if (!started || now < last) 
    { 
	last = now;
	started = 1;
	return;
    }*/

    uclock_t dt  = now - last;  last = now;
    if (dt > 100000) dt = 100000;         /* pause / get_ticks reset guard */
    acc += (long long)dt * conf.sample_rate;
    owed += (int)(acc / 1000000);  acc %= 1000000;

    int pos = 0;
    while (owed >= SLICE && pos + SLICE <= 2048) {
        YM2610Update_stream_to(SLICE, abuf + pos * 2);   /* your dst version */
        pos += SLICE;  owed -= SLICE;
    }

    if (owed > 4 * SLICE) owed = 4 * SLICE;  /* stalled: drop, don't burst */
    if (pos) {

	if (!audioStreamer->DirectWriteData((TUint8*)abuf, pos*4))
	{
	    fprintf(stderr, 
		    "SymbianAudioStream->DirectWriteDataWrite failed! err: %d\n",
		    audioStreamer->GetError());
	    delete audioStreamer;
	    audioStreamer = NULL;
	}
    }
}

#if 0
void SymbianAudioStream_Update()
{
    if (audioStreamer) {

	TInt nsamples = (conf.sample_rate/50);
	YM2610Update_stream(nsamples);
	if (!audioStreamer->DirectWriteData((TUint8*)play_buffer, nsamples*4)){
	    fprintf(stderr, 
		    "SymbianAudioStream->DirectWriteDataWrite failed! err: %d\n",
		    audioStreamer->GetError());
	    delete audioStreamer;
	    audioStreamer = NULL;
	}
    }
}
#endif 

void SymbianAudioStream_SetVolume(TInt aValue)
{

    if (audioStreamer) {
	audioStreamer->SetVolume(aValue);
    }
}

int init_sdl_audio(void)
{
    if (audioStreamer != NULL) {
	delete audioStreamer;
    }

    TRAPD(err, audioStreamer = CSymbianAudioStream::NewL(conf.sample_rate, 0));
    if (err != KErrNone) {
	fprintf(stderr, "SymbianAudioStream init error: %d", err);
	return 0;
    }

    int vol = CF_VAL(cf_get_item_by_name("audio_volume"));
    if (vol > 10) vol = 5;
    SymbianAudioStream_SetVolume(vol);
    return 1;
}

void pause_audio(int on)
{
}

void close_sdl_audio(void) 
{
    if (audioStreamer) {
	delete audioStreamer;
	audioStreamer = NULL;
    }
}



