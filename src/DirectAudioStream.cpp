
/*
 * File: DirectAudioStream.cpp
 * Initial boilerplate/template generated via Gemini.
 * Adapted, refactored, and maintained by JigokuMaster.
 */

#include "DirectAudioStream.h"
#include <e32base.h>
#include <e32std.h>
#include <stdio.h>


// Implementations of CSymbianAudioStream

CSymbianAudioStream* CSymbianAudioStream::NewL(TInt aSampleRate, TInt aBufferSize)
    {
        CSymbianAudioStream* self = new (ELeave) CSymbianAudioStream(aSampleRate, aBufferSize);
        CleanupStack::PushL(self);
        self->ConstructL();
        CleanupStack::Pop(self);
        return self;
    }


CSymbianAudioStream::CSymbianAudioStream(TInt aSampleRate, TInt aBufferSize) 
    : iSampleRate(aSampleRate),
    iBufferSize(aBufferSize)
    {
    }


CSymbianAudioStream::~CSymbianAudioStream()
    {
        if ( iStream ) {
            iStream->Stop();
            delete iStream;
            iStream = NULL;
        }

	iBuffer.Close();
    }

void CSymbianAudioStream::ConstructL()
    {

	iError = KErrNone;
        iVolume = 3;

	if (iBufferSize > 0)
	{
	    iBuffer.CreateL(iBufferSize);
	    iBuffer.SetMax();
	    iBuffer.FillZ();
	}

        iStream = CMdaAudioOutputStream::NewL(*this);
        iSettings.Query();
	iSettings.iCaps = TMdaAudioDataSettings::ERealTime |
     	TMdaAudioDataSettings::ESampleRateFixed; 
        iSettings.iSampleRate = TMdaAudioDataSettings::ESampleRate22050Hz;
        iSettings.iChannels = TMdaAudioDataSettings::EChannelsStereo;
        iSettings.iFlags = TMdaAudioDataSettings::ENoNetworkRouting;
        iSettings.iVolume = iSettings.iMaxVolume / 2;
        iStream->Open(&iSettings);
	CActiveScheduler::Start(); // wait for open.
	User::LeaveIfError(iError);
    }


TBool CSymbianAudioStream::WriteData(TUint8* aData, TInt aLen)
    {
	if ( iError == KErrNone ) {
	    iBuffer.Copy(aData, aLen);
	    TRAPD(iError, iStream->WriteL(iBuffer));
	    if (iError != KErrNone) return EFalse;	
	    CActiveScheduler::Start();

	}
	return ETrue;
    }

TBool CSymbianAudioStream::DirectWriteData(TUint8* aData, TInt aLen)
    {
	if ( iError == KErrNone ) {
	    iDataPtr.Set(aData, aLen);
	    TRAP(iError, iStream->WriteL(iDataPtr));
	    if (iError == KErrNone) {
		CActiveScheduler::Start();
	    }
	}
	return iError==KErrNone;
    }



void CSymbianAudioStream::SetVolume(TInt aNewVolume)
    {
	if (iStream)
	{
	    iVolume = Min(aNewVolume, 10);
            iStream->SetVolume((iStream->MaxVolume() * iVolume) / 10);
	}
    }


TBool CSymbianAudioStream::UpdateSndRate()
    {		
        TInt sampleRate = TMdaAudioDataSettings::ESampleRate22050Hz;

        if(iSampleRate == 8000) sampleRate = TMdaAudioDataSettings::ESampleRate8000Hz;
	else if(iSampleRate == 11025) sampleRate = TMdaAudioDataSettings::ESampleRate11025Hz;
        else if (iSampleRate == 16000) sampleRate = TMdaAudioDataSettings::ESampleRate16000Hz;
        else if (iSampleRate == 22050) sampleRate = TMdaAudioDataSettings::ESampleRate22050Hz;
        else if (iSampleRate == 32000) sampleRate = TMdaAudioDataSettings::ESampleRate32000Hz;
        else if (iSampleRate == 44100) sampleRate = TMdaAudioDataSettings::ESampleRate44100Hz;
  
	TRAPD(iError, iStream->SetDataTypeL(KMMFFourCCCodePCM16));

        TRAP(iError, iStream->SetAudioPropertiesL(sampleRate, TMdaAudioDataSettings::EChannelsStereo));

        return (iError == KErrNone);
    }

// MMdaAudioOutputStreamCallback Implementations
void CSymbianAudioStream::MaoscOpenComplete(TInt aError)
    {
	iError = aError;
        if (aError == KErrNone) {
            iStream->SetPriority(EPriorityMuchMore, EMdaPriorityPreferenceTimeAndQuality);        	    
            iStream->SetVolume((iStream->MaxVolume() * iVolume) / 10);

            if (UpdateSndRate()) {
                iIsOpen = ETrue;
            }
        }
        CActiveScheduler::Stop();
    }

void CSymbianAudioStream::MaoscBufferCopied(TInt aError, const TDesC8& aBuffer)
    {

	iError = aError;	
	CActiveScheduler::Stop();
    }

void CSymbianAudioStream::MaoscPlayComplete(TInt aError) 
    {
	iError = aError;
    }


