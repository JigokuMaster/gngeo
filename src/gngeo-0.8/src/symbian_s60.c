#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "symbian_s60.h"

char* g_symbian_gngeo_dir = "C:\\gngeo\\";
char* g_symbian_gngeo_romsdir = "C:\\gngeo\\roms\\";
char g_symbian_gngeo_datafile[256];
static int max_audio_volume = 256;
static int current_audio_volume = 0;

static void symbian_exit()
{
    getchar();
    exit(1);
}

static void symbian_mkdir(char* dir)
{
    DIR *dirptr = opendir(dir);
    if(dirptr != NULL)
    {
        closedir(dirptr);
    }
    else
    {
        mkdir(dir, 0777);
    }
    if(access(dir, F_OK | W_OK) == -1)
    {
	printf("Could not create path: %s\n", dir);
	symbian_exit(1);
    }
}

static int symbian_find_datafile()
{
    char* drive_paths[4] = {"F:\\", "E:\\", "C:\\", NULL};
    int i = 0;
    char* d;
    while((d = drive_paths[i]) != NULL)
    {
	if(access(d, F_OK | W_OK) == 0)
	{
	    sprintf(g_symbian_gngeo_datafile, "%sprivate\\%s\\gngeo_data.zip", d, GNGEO_APP_UID);
	    if(access(g_symbian_gngeo_datafile, F_OK) == 0){
		return 1;// found;
	    }
	}
	i++;
    }
    return 0;
}


void symbian_init()
{
    if(access("F:\\", F_OK | W_OK) == 0)
    {
        g_symbian_gngeo_dir = "F:\\gngeo\\";
        g_symbian_gngeo_romsdir = "F:\\gngeo\\roms\\";
    }
    
    else if(access("E:\\", F_OK | W_OK) == 0)
    {
        g_symbian_gngeo_dir = "E:\\gngeo\\";
        g_symbian_gngeo_romsdir = "E:\\gngeo\\roms\\";
    }

    symbian_mkdir(g_symbian_gngeo_dir);
    symbian_mkdir(g_symbian_gngeo_romsdir);
    setenv("HOME", g_symbian_gngeo_dir, 1); 
    chdir(g_symbian_gngeo_dir);
    symbian_mkdir("./screenshots");

    /* setup log files...*/
    int fd1 = open("stdout.log", O_WRONLY | O_CREAT | O_TRUNC);
    int fd2 = open("sterr.log", O_WRONLY | O_CREAT | O_TRUNC);
    dup2(fd1, 1); 
    dup2(fd2, 2);

    if(!symbian_find_datafile())
    {
	fprintf(stderr, "Could not find gngeo_data.zip\n");
	exit(1);
    }
    fprintf(stdout, "GNGEO_DIR=%s\n", g_symbian_gngeo_dir);
    fprintf(stdout, "GNGEO_ROMS_DIR=%s\n", g_symbian_gngeo_romsdir);
    fprintf(stdout, "GNGEO_DATAFILE=%s\n", g_symbian_gngeo_datafile);
}

char* symbian_gngeo_dir()
{
    return g_symbian_gngeo_dir;
}

char* symbian_gngeo_romsdir()
{
    return g_symbian_gngeo_romsdir;
}

char* symbian_gngeo_biosdir()
{
    return g_symbian_gngeo_romsdir;
}

char* symbian_gngeo_datafile()
{
    return g_symbian_gngeo_datafile;
}

void symbian_audio_volume_set(int v, int update)
{
    
    current_audio_volume = v;
    if(update)
    {
	current_audio_volume = EPOC_GetAudioVolume() + v;
	
    }

    if(current_audio_volume < 0)
    {
	current_audio_volume = 0;
    }
   
    if(current_audio_volume > max_audio_volume)
    {
	current_audio_volume = max_audio_volume;
    }
    EPOC_SetAudioVolume(current_audio_volume);
}

int symbian_audio_volume_get()
{
    return current_audio_volume;
}

void symbian_audio_mute()
{
    EPOC_SetAudioVolume(0);
}

int symbian_ui_orientation_get()
{
    return GetScreenOrientation();
}

int symbian_ui_orientation_setup()
{
    return SetupScreenOrientation();
}


