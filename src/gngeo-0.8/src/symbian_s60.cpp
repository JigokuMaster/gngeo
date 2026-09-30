
#include <aknappui.h>
#include <bautils.h>
#include <f32file.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "symbian_s60.h"

char g_symbian_gngeo_dir[12];
char g_symbian_gngeo_romsdir[17];
char g_symbian_gngeo_datafile[256];
static char* state_dir = "./state/";
static int current_audio_volume = 5;

static TBool GetPrivateFile(RFs& aRfs, TFileName &aFilePath, const TDesC &aFileName)
{
    if ( aRfs.PrivatePath(aFilePath) == KErrNone )
    {
	aFilePath.Insert(0, RProcess().FileName().Left(2)); // insert drive char + seperator
	aFilePath.Append(aFileName);
	return ETrue;
    }
    return EFalse;
}


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
	symbian_exit();
    }
}

static int symbian_init_dirs()
{
    char* priv_dir = getenv("EPOC_PRIVATE_DIR");
    if (!priv_dir) 
    {
	puts("EPOC_PRIVATE_DIR not found.\n");
	return 0;
    }

    sprintf(g_symbian_gngeo_datafile, "%s\\gngeo_data.zip", priv_dir);

    /* roms should be put in the drive where gngeo was installed */
    if(access(g_symbian_gngeo_datafile, F_OK | W_OK) == 0) 
    {
	sprintf(g_symbian_gngeo_dir, "%c:\\gngeo\\", priv_dir[0]);
	sprintf(g_symbian_gngeo_romsdir, "%sroms\\", g_symbian_gngeo_dir);
	symbian_mkdir(g_symbian_gngeo_dir);
	symbian_mkdir(g_symbian_gngeo_romsdir);
	return 1;
    }
    return 0;
}



void symbian_init()
{


    if(!symbian_init_dirs())
    {
	puts("Could not find gngeo path\n");
	symbian_exit();
    }

    setenv("HOME", g_symbian_gngeo_dir, 1); 
    chdir(g_symbian_gngeo_dir);    

    /* setup log files...*/
    // dup2 is buggy on Symbian OpenC layer ...
    freopen("stdout.log", "w+", stdout);
    freopen("sterr.log", "w+", stderr);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    fprintf(stdout, "GNGEO_DIR=%s\n", g_symbian_gngeo_dir);
    fprintf(stdout, "GNGEO_ROMS_DIR=%s\n", g_symbian_gngeo_romsdir);
    fprintf(stdout, "GNGEO_DATAFILE=%s\n", g_symbian_gngeo_datafile);
    symbian_mkdir("./screenshots");
    symbian_mkdir(state_dir);
    // copy the default config only if needed.
    RFs rfs;
    TInt error = KErrNone;
    if ((error = rfs.Connect()) == KErrNone) 
    {
       _LIT(KConfigFile, "gngeorc");
       TFileName source;
       TFileName target(_L("\\gngeo\\gngeorc"));
       target.Insert(0, RProcess().FileName().Left(2)); // insert drive char + seperator

       if (!BaflUtils::FileExists(rfs, target) && GetPrivateFile(rfs, source, KConfigFile))
       {

	   error = BaflUtils::CopyFile(rfs, source, target);
	   if (error == KErrNone) puts("gngeorc copied\n");
	   rfs.Close();
	}
    }

    if (error != KErrNone) {
	fprintf(stderr, "failed to copy gngeorc (%d)\n", error);
	exit(1);
    }
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


char* symbian_get_state_dir(char* game, int slot)
{
    if (!game) return state_dir; // flag to skip check if we are not reading the state file.

    char tmp_path[256] = {0,};
    sprintf(tmp_path, "%s%s.%03d", state_dir, game, slot);
    if(access(tmp_path, F_OK | W_OK) != -1) 
    {
	return state_dir;
    }
    return g_symbian_gngeo_dir; // old path for backward compatibly. 
}

char* symbian_get_nvram_dir(char* game)
{

    if (!game) return state_dir; // flag to skip check if we are not reading the state file.
    char tmp_path[256] = {0,};
    sprintf(tmp_path, "%s%s.nv", state_dir, game);
    if(access(tmp_path, F_OK | W_OK) != -1) 
    {
	return state_dir;
    }
    return g_symbian_gngeo_dir; // old path for backward compatibly.     
}


void symbian_audio_volume_set(int v, int update)
{

    int max_audio_volume = EPOC_GetAudioMaxVolume();

    if(update)
    {
	current_audio_volume += v;	
    }
    else {
	current_audio_volume = v;
    }

    if(current_audio_volume < 0)
    {
	current_audio_volume = 0;
    }
   
    if(current_audio_volume > max_audio_volume)
    {
	current_audio_volume = 5;
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

bool symbian_get_screenorientation()
{

    CAknAppUi* appUi = dynamic_cast<CAknAppUi*>(CEikonEnv::Static()->AppUi());
    if (!appUi) return 0;
    // return true if landscape is enabled.
    return ((appUi->ApplicationRect().Width() == 240) && (appUi->ApplicationRect().Height() == 320));   
}


extern TBool EPOC_SetupScreenOrientation();

bool symbian_setup_screenorientation()
{
    return EPOC_SetupScreenOrientation();
}



