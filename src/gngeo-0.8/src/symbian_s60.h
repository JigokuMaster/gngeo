#ifndef SYMBIAN_S60_H
#define SYMBIAN_S60_H

#define GNGEO_APP_UID "be99619c"

extern void EPOC_SetAudioVolume(int v);
extern int EPOC_GetAudioVolume();
extern int Epoc_GetAudioMaxVolume();
void symbian_init();
char* symbian_gngeo_dir();
char* symbian_gngeo_romsdir();
char* symbian_gngeo_biosdir();
char* symbian_gngeo_datafile();
void symbian_audio_volume_set(int v, int update);
int symbian_audio_volume_get();
int symbian_ui_orientation_get();
int symbian_ui_orientation_setup();

#endif
