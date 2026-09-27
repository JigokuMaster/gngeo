#include <e32base.h>
#include <eikapp.h>
#include <sdlapp.h>
#if defined (S60V3)
#include <eikenv.h>
#include <aknappui.h>
#endif
#include "SDLLauncher.hrh"
#if defined (UIQ3)
#include <ESDLTest.rsg>
#endif
#if defined (EPOC_AS_APP) && !defined (UIQ3) && !defined (S60V3)
#include "ECompXL.h"
#endif



#include <stdlib.h>
#include <stdio.h>
#ifndef OPENC
extern "C" void raise(int a)
{
    exit(a);
}
#else
extern "C" void _epoc32_atexit(void (*function)(void))
{
    atexit(function);
}

extern "C" void CloseSTDLIB(){} 
#endif


extern "C" int GetScreenOrientation()
{}


extern "C" void SetupScreenOrientation()
{}


LOCAL_C void SetEPOCEnvVars()
{
    char privDir[KMaxFileName] = {0,}; 
    char d = RProcess().FileName()[0];
    sprintf(privDir, "%c:\\private\\%08x\\", d, KSDLAppLauncherUid);
    setenv("EPOC_PRIVATE_DIR", privDir, 1);
}


class CESDLTestApp : public CSDLApp {
public:
	CESDLTestApp();
	~CESDLTestApp();
#if defined (UIQ3)
	/**
	 * Returns the resource id to be used to declare the views supported by this UIQ3 app
	 * @return TInt, resource id
	 */
	TInt ViewResourceId()
		{
		return R_SDL_VIEW_UI_CONFIGURATIONS;
		}

#endif


	void LaunchAppL(int argc, char** argv)
	{

	    /*char* myargv[2];
	    myargv[0] = argv[0];
	    myargv[1] = "C:\\Data\\SMSPlus\\sonic.sms";*/
	    SetEPOCEnvVars();
#ifdef S60V3
    
	    CAknAppUi* appUi = dynamic_cast<CAknAppUi*>(CEikonEnv::Static()->AppUi());
	    if (appUi) appUi->SetOrientationL(CAknAppUiBase::EAppUiOrientationPortrait);	    
#endif
	    CSDLApp::LaunchAppL(argc, argv);
	}


	TUid AppDllUid() const;
#if defined (EPOC_AS_APP) && !defined (UIQ3) && !defined (S60V3)
	TECompXL    iECompXL;
#endif
};

#ifdef EPOC_AS_APP

// this function is called automatically by the SymbianOS to deliver the new CApaApplication object
#if !defined (UIQ3) && !defined (S60V3)
EXPORT_C 
#endif
CApaApplication* NewApplication() {
	// Return pointer to newly created CQMApp
	return new CESDLTestApp;
}

#if defined (UIQ3) || defined (S60V3)
#include <eikstart.h>
// E32Main() contains the program's start up code, the entry point for an EXE.
GLDEF_C TInt E32Main() {
    return EikStart::RunApplication(NewApplication);
}
#endif

#endif // EPOC_AS_APP

#if !defined (UIQ3) && !defined (S60V3)
GLDEF_C  TInt E32Dll(TDllReason) {
	return KErrNone;
}
#endif
CESDLTestApp::CESDLTestApp() {	
}

CESDLTestApp::~CESDLTestApp() {
}

TUid CESDLTestApp::AppDllUid() const
{
	return  TUid::Uid(KSDLAppLauncherUid);
}


