#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "symbian_s60.h"

char* default_gngeo_dir = "c:\\gngeo\\";
char* default_gngeo_romsdir = "c:\\gngeo\\roms\\";
char* gngeo_datafile = NULL;
char* symbian_private_dir = NULL;

int init_dir(char* dir)
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
    return access(dir, F_OK | W_OK) == 0;
}


/*
    Hacks to access files  inside app private folder
*/

char* init_private_dir()
{
    char dir_buf[256];
    sprintf(dir_buf, "e:\\private\\%s\\",  GNGEO_APP_UID);
    if( access(dir_buf, F_OK | W_OK) == -1 )
    {
        sprintf(dir_buf, "c:\\private\\%s\\", GNGEO_APP_UID);
    }

    return dir_buf;
}

char* get_private_file(char* fp)
{

    static char symbian_private_file[256] = "";
    sprintf(symbian_private_file, "%s\\%s", symbian_private_dir, fp);
    if(symbian_private_file)
    {
        return symbian_private_file;
    }
    return fp;
}


void symbian_init()
{

    /* better to let user manually choose all those paths*/
    
    if(access("e:\\", F_OK | W_OK) == 0)
    {
        default_gngeo_dir = "e:\\gngeo\\";
        default_gngeo_romsdir = "e:\\gngeo\\roms\\";
    }
    
    if(init_dir(default_gngeo_dir))
    {

        init_dir(default_gngeo_romsdir);
    }
    
    symbian_private_dir = init_private_dir(); /* \\private\\app_uid */
    gngeo_datafile = get_private_file("gngeo_data.zip"); /* user should put it in default_gngeo_dir, if something went wrong ? */
    setenv("HOME", default_gngeo_dir, 1); 
    chdir(default_gngeo_dir); /* move to the public folder*/

    /* setup log files...*/
    int fd1 = open("stdout.log", O_WRONLY | O_CREAT | O_TRUNC);
    int fd2 = open("sterr.log", O_WRONLY | O_CREAT | O_TRUNC);
    dup2(fd1, 1); 
    dup2(fd2, 2);
    
    /* test logging ...*/
    fprintf(stdout, "GNGEO_PRIVATE_DIR=%s\n", symbian_private_dir);
    fprintf(stdout, "GNGEO_DIR=%s\n", default_gngeo_dir);
    fprintf(stdout, "GNGEO_ROMS_DIR=%s\n", default_gngeo_romsdir);
    fprintf(stdout, "GNGEO_DATAFILE=%s\n", gngeo_datafile);
}


char* symbian_gngeo_dir()
{
    return default_gngeo_dir;
}

char* symbian_gngeo_romsdir()
{
    return default_gngeo_romsdir;
}

char* symbian_gngeo_biosdir()
{
    return default_gngeo_romsdir;
}

char* symbian_gngeo_datafile()
{
    return gngeo_datafile;
}

void symbian_audio_volume_set(int v, int update)
{
    if(update)
    {
	EPOC_SetAudioVolume(EPOC_GetAudioVolume() + v);
    }
    else
    {
	EPOC_SetAudioVolume(v);
    }
}

int symbian_audio_volume_get()
{
    return EPOC_GetAudioVolume();
}

int symbian_ui_orientation_get()
{
    return GetScreenOrientation();
}

int symbian_ui_orientation_setup()
{
    return SetupScreenOrientation();
}


