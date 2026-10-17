

#ifndef DIRECT_AUDIO_H
#define DIRECT_AUDIO_H

#include <mdaaudiooutputstream.h>
#include <mda/common/audio.h>

class CSymbianAudioStream : public CBase, public MMdaAudioOutputStreamCallback
{
public:
    static CSymbianAudioStream* NewL(TInt aSampleRate, TInt aBufferSize);
    ~CSymbianAudioStream();
    TBool WriteData(TUint8* aData, TInt aLen);
    TBool DirectWriteData(TUint8* aData, TInt aLen);
    TBool UpdateSndRate();
    void SetVolume(TInt aNewVolume);
    TInt GetError() { return iError; }

    // from MMdaAudioOutputStreamCallback
    virtual void MaoscOpenComplete(TInt aError);
    virtual void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);
    virtual void MaoscPlayComplete(TInt /*aError*/);

private:
    CSymbianAudioStream(TInt aSampleRate, TInt aBufferSize);
    void ConstructL();
    TBool                   iIsOpen;
    int                     iSampleRate;
    CMdaAudioOutputStream*  iStream;
    TMdaAudioDataSettings   iSettings;
    TInt                    iVolume;
    TInt 		    iError;
    RBuf8	    	    iBuffer;
    TInt 		    iBufferSize;
    TPtrC8	    	    iDataPtr;
};

#endif // DIRECT_AUDIO_H


