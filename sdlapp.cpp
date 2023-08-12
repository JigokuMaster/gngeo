#include <coecntrl.h>
#include <aknappui.h>
#include <aknapp.h>
#include <akndoc.h>
#include <sdlepocapi.h>
#include <aknnotewrappers.h>
#include <eikstart.h>


const TUid KUidSdlApp={0xbe99619c};


class MExitWait
 	{
 	public:
 		virtual void DoExit(TInt aErr) = 0;
 	};   

class CExitWait : public CActive
 	{
 	public:
 		CExitWait(MExitWait& aWait);
 		void Start();
 		~CExitWait();
 	private:
 		void RunL();
 		void DoCancel();
 	private:
 		MExitWait& iWait;
 		TRequestStatus* iStatusPtr;
 	};

class CSDLWin : public CCoeControl
	{
	public:
		void ConstructL(const TRect& aRect);
		RWindow& GetWindow() const;
		void SetNoDraw();
	private:
		void Draw(const TRect& aRect) const;
	}; 	
	
class CSdlApplication : public CAknApplication
	{
private:
	// from CApaApplication
	CApaDocument* CreateDocumentL();
	TUid AppDllUid() const;
	};
	
	
class CSdlAppDocument : public CAknDocument
	{
public:
	CSdlAppDocument(CEikApplication& aApp): CAknDocument(aApp) { }
private:
	CEikAppUi* CreateAppUiL();
	};
	
		
class CSdlAppUi : public CAknAppUi, public MExitWait, MSDLObserver
{

public:
    void ConstructL();
    ~CSdlAppUi();
private:
    void HandleCommandL(TInt aCommand);
    void HandleForegroundEventL(TBool aForeground);
    void StartSDLApp();
    void DoExit(TInt aErr);
    TInt SdlEvent(TInt aEvent, TInt aParam);
    TInt SdlThreadEvent(TInt aEvent, TInt aParam);


    void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);
private:
	CExitWait* iWait;
	CSDLWin* iSDLWin;
	CSDL* iSdl;
	TBool iExit;
  	};	


CExitWait::CExitWait(MExitWait& aWait) : CActive(CActive::EPriorityStandard), iWait(aWait)
	{
	CActiveScheduler::Add(this);
	}
	
CExitWait::~CExitWait()
	{
	Cancel();
	}
 
void CExitWait::RunL()
	{
	if(iStatusPtr != NULL )
		iWait.DoExit(iStatus.Int());
	}
	
void CExitWait::DoCancel()
	{
	if(iStatusPtr != NULL )
		User::RequestComplete(iStatusPtr , KErrCancel);
	}
	
void CExitWait::Start()
	{
	SetActive();
	iStatusPtr = &iStatus;
	}

void CSDLWin:: ConstructL(const TRect& aRect)	
	{
	CreateWindowL();
	SetRect(aRect);
	ActivateL();
	}

	
RWindow& CSDLWin::GetWindow() const
	{
	return Window();
	}
	

void CSDLWin::Draw(const TRect& /*aRect*/) const
	{
	CWindowGc& gc = SystemGc();
	gc.SetPenStyle(CGraphicsContext::ESolidPen);
	gc.SetPenColor(KRgbGray);
	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	gc.SetBrushColor(0xaaaaaa);
	gc.DrawRect(Rect());
	}	
	
void CSdlAppUi::ConstructL()	
{
	BaseConstructL(CAknAppUi::EAknEnableSkin /* | ENoScreenFurniture*/ );
	// SetOrientationL(CAknAppUiBase::EAppUiOrientationLandscape);
	iSDLWin = new (ELeave) CSDLWin;
	iSDLWin->ConstructL(ApplicationRect());
	iWait = new (ELeave) CExitWait(*this);
  	StartSDLApp();
}


void CSdlAppUi::HandleForegroundEventL(TBool aForeground)
{
    CAknAppUi::HandleForegroundEventL(aForeground);
}


void CSdlAppUi::HandleCommandL(TInt aCommand)
{
	switch(aCommand)
	{
		case EAknSoftkeyExit:
		case EAknSoftkeyBack:
		case EAknCmdExit:
    	case EEikCmdExit:
    	    if(iWait == NULL || !iWait->IsActive() || iSdl == NULL)
		    {
		        Exit();
		    }
		    if(!iExit)
		    {
		        TWsEvent event;
	    	    event.SetType(EEventSwitchOff),
		        event.SetTimeNow();
	    	    iSdl->AppendWsEvent(event);
	    	    User::After(1000000);
     		    iExit = ETrue;
     		    iSdl->Suspend();
		        Exit();
     	    }
		    break;
		
	}
}


TInt CSdlAppUi::SdlThreadEvent(TInt aEvent, TInt /*aParam*/)    
{
    switch(aEvent)
	{
	case MSDLObserver::EEventResume:
	    break;
	case MSDLObserver::EEventSuspend:
	    if(iExit)
	    {
		return MSDLObserver::ESuspendNoSuspend;
	    }
	    break;
	case MSDLObserver::EEventWindowReserved:
	    break;
	case MSDLObserver::EEventWindowNotAvailable:
	    break;
	case MSDLObserver::EEventScreenSizeChanged:
            break;
	}
    return MSDLObserver::EParameterNone;	
    }


TInt CSdlAppUi::SdlEvent(TInt aEvent, TInt /*aParam*/)    
{

    switch(aEvent)
    {
	case MSDLObserver::EEventResume:
	    break;
	case MSDLObserver::EEventSuspend:
	    if(iExit)
	    {
		return MSDLObserver::ESuspendNoSuspend;
	    }
	    break;
	case MSDLObserver::EEventWindowReserved:
	    break;
	case MSDLObserver::EEventWindowNotAvailable:
	    break;
	case MSDLObserver::EEventScreenSizeChanged:
    	break;
	case MSDLObserver::EEventKeyMapInit:
	    break;
	case MSDLObserver::EEventMainExit:
	    break;
	}
	
    return MSDLObserver::EParameterNone;

}



void CSdlAppUi::StartSDLApp()
	{
  	TInt flags = CSDL::ENoFlags | CSDL::EAllowImageResize;
  	iSdl = CSDL::NewL(flags);
  	iSdl->SetObserver(this);
  	iSdl->DisableKeyBlocking(*this);
 	iSdl->SetContainerWindowL(
 					iSDLWin->GetWindow(), 
        			iEikonEnv->WsSession(),
        			*iEikonEnv->ScreenDevice());
	
  	iSdl->CallMainL(iWait->iStatus);
  	iWait->Start();
	}

void CSdlAppUi::DoExit(TInt aErr)
{
	if(aErr != KErrNone)
	{
		CAknErrorNote* err = new (ELeave) CAknErrorNote(ETrue);
		TBuf<64> buf;
		buf.Format(_L("SDL Error %d"), aErr);
		err->ExecuteLD(buf);
	}	

	delete iSdl;
	iSdl = NULL;
	iExit = ETrue;
	Exit();
}
	
void CSdlAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
{
    const TInt type = aEvent.Type();
 	if(iSdl != NULL)
 	{

    if((type == EEventKey || type == EEventKeyUp || type == EEventKeyDown))
    {
        TWsEvent event;
        event.SetType(type);
        const TKeyEvent& key = *aEvent.Key();
        if(key.iScanCode == EStdKeyDevice0 || key.iScanCode == EStdKeyDevice1)
        {
            iSdl->AppendWsEvent(aEvent);
            return;
        }

     }
     
      iSdl->AppendWsEvent(aEvent);
      
     }
 	CAknAppUi::HandleWsEventL(aEvent, aDestination);
 }
 		
CSdlAppUi::~CSdlAppUi()
	{
	if(iWait != NULL)
	{
		iWait->Cancel();
	}
	delete iSdl;
	delete iWait;
	delete iSDLWin;
	}

CEikAppUi* CSdlAppDocument::CreateAppUiL()
    {
    return new(ELeave) CSdlAppUi();
    }	
	
TUid CSdlApplication::AppDllUid() const
    {
    return KUidSdlApp;
    }	
    

CApaDocument* CSdlApplication::CreateDocumentL()
    {
    CSdlAppDocument* document = new (ELeave) CSdlAppDocument(*this);
    return document;
    }
  
LOCAL_C CApaApplication* NewApplication()
    {
    return new CSdlApplication;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }
    

