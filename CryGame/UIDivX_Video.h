//-------------------------------------------------------------------------------------------------
// Author: Ivo Herzeg
//
// Purpose:
//  - A DivX-Video Control
//
// History:
//  - [19/2/2004] created the file
//
//-------------------------------------------------------------------------------------------------

#ifndef UIDivX_Video_H
#define UIDivX_Video_H

#if !defined(LINUX) && !defined(NOT_USE_DIVX_SDK)

#ifdef WIN64
#pragma comment(lib, "DivXMediaLib64.lib")
#else
#pragma comment(lib, "DivXMediaLib32.lib")
#endif

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

#include <dsound.h>



#define MAXSBUF (0x200000)  //2Mbyte for sound


//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define NUM_PLAY_NOTIFICATIONS  4
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }



//-----------------------------------------------------------------------------
// DivX functions and structures
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
 
typedef int HDIVX;
typedef int DIVX_RET;


//return values
#define DIVX_OK				0
#define DIVX_FAIL			1
#define MODE_SET_ERROR		2
#define WRONG_MODE			3	//returned when a functionality not related to the mode in which the decoder is, used.
#define NOT_SUPPROTED		4	//returned when a functionality is not supported
#define INVALID_HANDLE		5	//returned when a invalid handle is passed.

#define NO_FRAMES			101

#define NO_CHUNKS			102

//macros
#define MAX_ERR_MSG_LENGTH		200	//maximum length of error messages.
#define MAX_NUM_VID_BUFFERS		20	//maximum number of video buffers
#define MAX_NUM_AUD_BUFFERS		20	//maximum number of audio buffers.
#define MAX_AUD_BUFFER_SIZE		(1024*1024*10)	//maximum audio buffersize.


//Error values that DivxGetLastError returns
#define ERR_BASE						1000
#define ERR_VAL_NO_ERROR				ERR_BASE

#define ERR_VAL_NOT_AVI_FILE			ERR_VAL_NO_ERROR				+ 1	//1001
#define ERR_VAL_AVI_NO_HDRL				ERR_VAL_NOT_AVI_FILE			+ 1
#define ERR_VAL_AVI_NO_MOVI				ERR_VAL_AVI_NO_HDRL				+ 1
#define ERR_VAL_AVI_NO_VIDS				ERR_VAL_AVI_NO_MOVI				+ 1

#define ERR_VAL_UNEXPECTED_ERROR		ERR_VAL_AVI_NO_VIDS				+ 1 //1005
#define ERR_VAL_READ_FILE_FAILED		ERR_VAL_UNEXPECTED_ERROR		+ 1
#define ERR_VAL_INVALID_HANDLE			ERR_VAL_READ_FILE_FAILED		+ 1
#define ERR_VAL_OUT_OF_MEMORY			ERR_VAL_INVALID_HANDLE			+ 1
#define ERR_VAL_FRAME_MODE_CHANGE		ERR_VAL_OUT_OF_MEMORY			+ 1
#define ERR_VAL_NO_FRAMES				ERR_VAL_FRAME_MODE_CHANGE		+ 1
#define ERR_VAL_PLAYING_ALREADY			ERR_VAL_NO_FRAMES				+ 1
#define ERR_VAL_VID_BUF_NULL			ERR_VAL_PLAYING_ALREADY			+ 1
#define ERR_VAL_AUD_BUF_NULL			ERR_VAL_VID_BUF_NULL			+ 1
#define ERR_VAL_FUN_USED_IN_WRONG_MODE	ERR_VAL_AUD_BUF_NULL			+ 1
#define ERR_VAL_PLAY_TO_PAUSE			ERR_VAL_FUN_USED_IN_WRONG_MODE	+ 1 //1015
#define ERR_VAL_PAUSE_TO_RESUME			ERR_VAL_PLAY_TO_PAUSE			+ 1
#define ERR_VAL_INVALID_FRAME_NUMBER	ERR_VAL_PAUSE_TO_RESUME			+ 1
#define ERR_VAL_NO_FRAME_FOR_GIVEN_TIME	ERR_VAL_INVALID_FRAME_NUMBER	+ 1
#define ERR_VAL_NO_AUDIO_CHUNKS_LEFT	ERR_VAL_NO_FRAME_FOR_GIVEN_TIME	+ 1
#define ERR_VAL_FREE_PLAY_STOP_UNEXPEC	ERR_VAL_NO_AUDIO_CHUNKS_LEFT	+ 1
#define ERR_VAL_MAX_FILES_OPENED		ERR_VAL_FREE_PLAY_STOP_UNEXPEC	+ 1
#define ERR_VAL_OPEN_FILE_FAILED		ERR_VAL_MAX_FILES_OPENED		+ 1
#define ERR_VAL_FILE_NOT_FOUND			ERR_VAL_OPEN_FILE_FAILED		+ 1
#define ERR_VAL_NOT_DXGM				ERR_VAL_FILE_NOT_FOUND			+ 1 //1024
#define ERR_VAL_IO_NOT_INIT				ERR_VAL_NOT_DXGM				+ 1 




typedef enum EMode_tag
{
	eFREE_PLAY,
	eFRAME
}EMode;

typedef enum EOutputVidFormat_tag
{
	eRGB_24BIT,			//24-bit RGB
	eRGB_32BIT,			//32-bit RGB		(Alpha plane set to 0)
 	eRGB_16BIT_555,		//16-bit RGB (555)
	eRGB_16BIT_565,		//16-bit RGB (565)
	eARGB_32BIT,		//32-bit RGB		(Alpha plane set to 255)
	eABGR_24BIT,		//24-bit BGR		(OpenGL)
	eABGR_32BIT,		//32-bit BGR		(OpenGL)
	eYUV2,				//packed 4:2:2 YUV, order Y-U-Y-V
	eUYVY,				//packed 4:2:2 YUV, order U-Y-V-Y
	eIYUV,				//Planar 4:2:0 YUV, order Y-U-V
	eYV12				//Planar 4:2:0 YUV, order Y-V-U
}EOutputVidFormat;

typedef enum EProfile_tag
{
	ePROFILE_ENABLE,
	ePROFILE_DISABLE
}EProfile;

typedef struct
{
	int width;
	int height;
	double videoFrameRate;
	unsigned int numVideoFrames ;
	long   video_pos;         // Number of next frame to be read (if index present)
	long   a_fmt;             // Audio format, 
	long   a_chans;           // Audio channels, 0 for no audio
	long   a_rate;            // Rate in Hz
	long   a_bits;            // bits per audio sample
	long   audio_strn;        // Audio stream number
	long   audio_bytes;       // Total number of bytes of audio data
	long   audio_chunks;      // Chunks of audio data in the file
	long   audio_posc;        // Audio position: chunk
}Content_Info;


#ifdef DIVXMEDIALIB_EXPORTS
	#define FUN_DECL __declspec( dllexport ) 
#else
	#define FUN_DECL __declspec( dllimport ) __cdecl
#endif
	

//File operations
HDIVX FUN_DECL OpenDivxFile(const char * szFileName);

void FUN_DECL CloseDivxFile(HDIVX filehandle);

//mode setting
DIVX_RET FUN_DECL DivxModeSet(HDIVX handle,int mode);

//FreePlay functions
DIVX_RET FUN_DECL DivxPlay(HDIVX handle);

DIVX_RET FUN_DECL DivxStop(HDIVX handle);

DIVX_RET FUN_DECL DivxPause(HDIVX handle);

DIVX_RET FUN_DECL DivxResume(HDIVX handle);

//Frame access
DIVX_RET FUN_DECL DivxDoFrame(HDIVX handle);

DIVX_RET FUN_DECL DivxNextFrame(HDIVX handle);

DIVX_RET FUN_DECL DivxGotoFrame(HDIVX handle,long frame);

DIVX_RET FUN_DECL DivxGotoFrameEx(HDIVX handle,unsigned long milliSec,long *frameNo);

//others
DIVX_RET FUN_DECL DivxSetCallBackFn(HDIVX handle,const void* DivxWaitptr);
//Call back function
//void DivxWait();

DIVX_RET FUN_DECL DivxSetVideoBuffer(HDIVX handle,void *vidBuffer);

DIVX_RET FUN_DECL DivxSetAudioBuffer(HDIVX handle,void *audBuffer);

DIVX_RET FUN_DECL DivxSetNumAudVidBuffers(int numVidBuf,int numAudBuf,long audBufferSize);

DIVX_RET FUN_DECL DivxGetContentInfo(HDIVX handle,Content_Info* contentInfo);

DIVX_RET FUN_DECL DivxGetSDKVersion(int *version);

DIVX_RET FUN_DECL DivxProfile(HDIVX hHandle,EProfile eProfile);

DIVX_RET FUN_DECL DivxSetOutputVideoFormat(HDIVX hHandle,EOutputVidFormat eOutputVidFormat);

//Audio
DIVX_RET FUN_DECL DivxNextAudioChunk(HDIVX handle,int* actualBytes);

//Error functions
DIVX_RET FUN_DECL DivxGetLastError(HDIVX handle,long *errorNum);

DIVX_RET FUN_DECL DivxFormatErrorMessage(HDIVX handle,long errorNum,char* errMessage,int errMsgLength);

DWORD WINAPI NotificationProc( LPVOID lpParameter );

#ifdef __cplusplus
}
#endif






//-----------------------------------------------------------------------------
// Classes used by this header
//-----------------------------------------------------------------------------
class CUIVideoPanel;
class CSoundManager;
class CSound;
class CStreamingSound;
class CWaveFile;




//-----------------------------------------------------------------------------
// Typing macros 
//-----------------------------------------------------------------------------
#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

#define DSUtil_StopSound(s)         { if(s) s->Stop(); }
#define DSUtil_PlaySound(s)         { if(s) s->Play( 0, 0 ); }
#define DSUtil_PlaySoundLooping(s)  { if(s) s->Play( 0, DSBPLAY_LOOPING ); }




//-----------------------------------------------------------------------------
// Name: class CWaveFile
// Desc: Encapsulates reading or writing sound data to or from a wave file
//-----------------------------------------------------------------------------
class CWaveFile
{

	public:
		WAVEFORMATEX m_wfx; 
    MMCKINFO      m_ck;          // Multimedia RIFF chunk
    MMCKINFO      m_ckRiff;      // Use in opening a WAVE file
    DWORD         m_dwSize;      // The size of the wave file

		//------------------------------------------------------------

		CWaveFile()	{  m_dwSize  = 0; }
		~CWaveFile()	{}

		DWORD GetSize() {  return m_dwSize; }
		HRESULT ResetFile();
};



//-----------------------------------------------------------------------------
// Name: class CSoundManager
// Desc: 
//-----------------------------------------------------------------------------
class CSoundManager
{
public:
    LPDIRECTSOUND8 m_pDS;
		CWaveFile						m_WaveFile;

		CSoundManager() { m_pDS = NULL; }
		~CSoundManager() { SAFE_RELEASE( m_pDS ); }

    HRESULT Initialize( HWND hWnd, DWORD dwCoopLevel );
    HRESULT SetPrimaryBufferFormat( DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate );
    HRESULT CreateStreaming( DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent );
};





//-----------------------------------------------------------------------------
// Name: class CStreamingSound
// Desc: Encapsulates functionality to play a wave file with DirectSound.  
//       The Create() method loads a chunk of wave file into the buffer, 
//       and as sound plays more is written to the buffer by calling 
//       HandleWaveStreamNotification() whenever hNotifyEvent is signaled.
//-----------------------------------------------------------------------------
class CStreamingSound
{
protected:
		LPDIRECTSOUNDBUFFER* m_apDSBuffer;
    DWORD                m_dwDSBufferSize;
    DWORD                m_dwNumBuffers;
    DWORD                m_dwCreationFlags;
	
		DWORD			m_dwLastPlayPos;
    DWORD			m_dwPlayProgress;
    DWORD			m_dwNotifySize;
    DWORD			m_dwNextWriteOffset;
    BOOL			m_bFillNextNotificationWithSilence;

public:
		u32 m_SoundEnabled;
		//-----------------------------------------------------------------------------
		// Name: CStreamingSound::CStreamingSound()
		// Desc: Setups up a buffer so data can be streamed from the wave file into 
		//       a buffer.  This is very useful for large wav files that would take a 
		//       while to load.  The buffer is initially filled with data, then 
		//       as sound is played the notification events are signaled and more data
		//       is written into the buffer by calling HandleWaveStreamNotification()
		//-----------------------------------------------------------------------------
		CStreamingSound()	{}
		~CStreamingSound() { Release();		};

		void Release() {
			for( DWORD i=0; i<m_dwNumBuffers; i++ )
			{
        SAFE_RELEASE( m_apDSBuffer[i] ); 
			}
			SAFE_DELETE_ARRAY( m_apDSBuffer ); 
		}


    HRESULT RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored );





		void Init( LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize,CWaveFile* pWaveFile, DWORD dwNotifySize ) 
		{
			m_apDSBuffer = new LPDIRECTSOUNDBUFFER[1];
			if( NULL != m_apDSBuffer )
			{
        for( DWORD i=0; i<1; i++ ) m_apDSBuffer[i] = &pDSBuffer[i];
        m_dwDSBufferSize	= dwDSBufferSize;
        m_dwNumBuffers		= 1;
        m_dwCreationFlags = 0;
			}

			m_SoundEnabled			=	0;
			m_dwLastPlayPos     = 0;
			m_dwPlayProgress    = 0;
			m_dwNotifySize      = dwNotifySize;
			m_dwNextWriteOffset = 0;
			m_bFillNextNotificationWithSilence = FALSE;
		}


    HRESULT FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger );

		LPDIRECTSOUNDBUFFER GetFreeBuffer();
    LPDIRECTSOUNDBUFFER GetBuffer( DWORD dwIndex );

    HRESULT Play( DWORD dwPriority = 0, DWORD dwFlags = 0, LONG lVolume = 0, LONG lFrequency = -1, LONG lPan = 0 );
    HRESULT Stop();

		HRESULT HandleWaveStreamNotification( BOOL bLoopedPlay );
    HRESULT Reset();
};



//-----------------------------------------------------------------------------
// Name: class CDivXPlayer
// Desc: 
//-----------------------------------------------------------------------------

class CDivXPlayer {

	public:

		CSoundManager       m_SoundManager;
		CStreamingSound     m_StreamingSound;

		HANDLE		m_hNotificationEvent;
		DWORD			m_dwNotifyThreadID;
		HANDLE		m_hNotifyThread;
		HINSTANCE	m_hInst;

		f32 m_VideoTime;
		u32 m_FrameCounter;
		u32	m_AudioCounter;

		uint8* m_pFrameBuffer;
		uint8* m_pAudioBuffer;

		HDIVX divxFile;
		Content_Info divxFileInfo;
		DIVX_RET retVal;

		u32 m_LastFramer;

	//------------------------------------------

		CDivXPlayer(){
			m_hNotificationEvent    = NULL;
			m_dwNotifyThreadID      = 0;
			m_hNotifyThread         = NULL;
			m_hInst                 = NULL;

			m_VideoTime				=	0.0f;
			m_FrameCounter		=	0;
			m_AudioCounter		=	0;
			m_pFrameBuffer		= NULL;
			m_pAudioBuffer		= NULL;

			m_LastFramer			=	0xffffffff;
		};
		~CDivXPlayer(){};

		bool Load_DivX( CUIVideoPanel* pPanel, const string &szFileName );
		void Stop_DivX();	
		bool Update_DivX( CUIVideoPanel* pPanel );
		HRESULT StopSound();
		HRESULT PlaySound();

		f64 CDivXPlayer::GetQPF() {
			LARGE_INTEGER nFreq;
			QueryPerformanceFrequency(&nFreq);
			return (f64)nFreq.QuadPart;
		}

		f64 CDivXPlayer::GetQPC() {
			LARGE_INTEGER nTime;
			QueryPerformanceCounter(&nTime);
			return (f64)nTime.QuadPart;
		}

};


extern CDivXPlayer g_DivXPlayer;	

#endif

#endif 
