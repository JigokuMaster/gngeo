#ifndef SYMBIAN_S60_H
#define SYMBIAN_S60_H

#define GNGEO_APP_UID "be99619c"

extern void EpocAudioMaxVolume();
extern void EpocAudioSetVolume(int v);
extern int EpocAudioGetVolume();
void symbian_init();
char* symbian_gngeo_dir();
char* symbian_gngeo_romsdir();
char* symbian_gngeo_biosdir();
char* symbian_gngeo_datafile();
void symbian_audio_volume(int v, int update);

#endif
