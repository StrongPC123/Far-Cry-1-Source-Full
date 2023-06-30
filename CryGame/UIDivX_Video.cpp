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

#define STRICT
#include "StdAfx.h"

#if !defined(LINUX) &&  !defined(NOT_USE_DIVX_SDK)

#include <Cry_Math.h>
#include <windows.h>
#include "UIDivX_Video.h"
#include "UIVideoPanel.h"






//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
CDivXPlayer					g_DivXPlayer;	







bool CDivXPlayer::Load_DivX( CUIVideoPanel* pPanel, const string &szFileName ) {

	//stop playing
	pPanel->m_bPaused = 0;
	pPanel->m_bPlaying = 0;

	if (szFileName.empty())
	{
		return 0;
	}

	//convert ".bik" to ".avi"
	string FileName=szFileName;
	size_t size=FileName.size();
	FileName[size-4]=(char)'.';
  FileName[size-3]=(char)'a';
  FileName[size-2]=(char)'v';
  FileName[size-1]=(char)'i';

	//just in case there is already an AVI playing
	Stop_DivX();

	//--------------------------------------------------------
	//video init
	//--------------------------------------------------------
	divxFile = OpenDivxFile( FileName.c_str() );

	if (divxFile==0) {
		pPanel->GetUISystem()->GetISystem()->GetILog()->Log("DivX: failed to load: %s", FileName.c_str() );
		pPanel->OnError("AVI not found");
		return 0;
	}	else {
		pPanel->GetUISystem()->GetISystem()->GetILog()->Log("DivX: loading file: %s", FileName.c_str()  );
	}
	retVal = DivxGetContentInfo( divxFile, &divxFileInfo ); //learn details about the file
	assert(!retVal);
	
	//allocate a frame buffer
	uint32 fbsize=(divxFileInfo.height * divxFileInfo.width * 4 * 2);
	m_pFrameBuffer = new unsigned char [fbsize];	
	assert(m_pFrameBuffer);
	uint64* fb=(uint64*)m_pFrameBuffer; 
	for(uint32 x=0; x<(fbsize/8); x++) fb[x]=0;

	//set output to openGL 32 bit aligned data
	retVal = DivxSetOutputVideoFormat( divxFile, eARGB_32BIT );
	assert(!retVal);

	//set up the callback function
	retVal = DivxSetCallBackFn( divxFile, 0 ); //you have to work out some way to let this guy know about the frame buffer.
	assert(!retVal);

	//give the codec the output buffers
	retVal = DivxSetVideoBuffer( divxFile, (void*)m_pFrameBuffer );
	assert(!retVal);

	retVal = DivxModeSet( divxFile, eFRAME );
	assert(!retVal);


	//-----------------------------------------------------------------
	//audio init
	//-----------------------------------------------------------------
	if (divxFileInfo.audio_bytes) { 

		HWND hwnd = (HWND)pPanel->GetUISystem()->GetISystem()->GetIRenderer()->GetHWND();

		if (m_pAudioBuffer==0)	m_pAudioBuffer = new unsigned char [MAX_AUD_BUFFER_SIZE];	//allocate the audio buffer
		for(uint32 x=0; x<MAX_AUD_BUFFER_SIZE; x++) { m_pAudioBuffer[x]=0; };
		assert( m_pAudioBuffer );

		retVal = DivxSetAudioBuffer( divxFile, (void*)m_pAudioBuffer );
		assert(!retVal);

		m_hNotificationEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
		assert(g_DivXPlayer.m_hNotificationEvent);

		//Create a static IDirectSound in the CSound class.  
		HRESULT hr = g_DivXPlayer.m_SoundManager.Initialize( hwnd, DSSCL_PRIORITY );
	
		if (hr==S_OK) {
			// Create a thread to handle DSound notifications
			m_hNotifyThread = CreateThread( NULL, 0, NotificationProc, hwnd, 0, &m_dwNotifyThreadID );
			// Create a timer, so we can check for when the soundbuffer is stopped
			SetTimer( hwnd, 0, 250, NULL );

			//-----------------------------------------------------------------------------
			// create a DirectSound buffer.  
			// Since we are streaming data into the buffer, the buffer will be filled with data 
			// when the sound is played, and as notification events are signaled
			//-----------------------------------------------------------------------------
			// This samples works by dividing a 3 second streaming buffer into 
			// NUM_PLAY_NOTIFICATIONS (16) pieces.  It creates a notification for each
			// piece and when a notification arrives then it fills the circular streaming 
			// buffer with new wav data over the sound data which was just played
			// Determine the g_dwNotifySize.  It should be an integer multiple of nBlockAlign
			DWORD nBlockAlign		= 4;
			INT nSamplesPerSec	= 44100;
			DWORD dwNotifySize	= nSamplesPerSec * 3 * nBlockAlign / NUM_PLAY_NOTIFICATIONS;
			dwNotifySize				-= dwNotifySize % nBlockAlign;   

			// Set up the direct sound buffer.  Request the NOTIFY flag, so
			// that we are notified as the sound buffer plays.  Note, that using this flag
			// may limit the amount of hardware acceleration that can occur. 
			HRESULT hr = g_DivXPlayer.m_SoundManager.CreateStreaming(  DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2, GUID_NULL, NUM_PLAY_NOTIFICATIONS, dwNotifySize, m_hNotificationEvent );
			assert(hr==S_OK);
		} 

}


	//-----------------------------------------------------------------------------
	// create texture for blitting
  // WORKAROUND: NVidia driver bug during playing of video file
  // Solution: Never remove video texture (non-power-of-two)
	//	m_pUISystem->GetIRenderer()->UpdateTextureInVideoMemory(m_iTextureID, (uint8*)m_pFrameBuffer, 0, 0, divxFileInfo.width, divxFileInfo.height, eTF_8888);

	if (pPanel->m_iTextureID > -1)
	{
		pPanel->GetUISystem()->GetIRenderer()->RemoveTexture(pPanel->m_iTextureID);
		pPanel->m_iTextureID = -1;
	}
	
	if (divxFileInfo.width==640 && divxFileInfo.height==480)
	  pPanel->m_iTextureID = 	pPanel->GetUISystem()->GetIRenderer()->DownLoadToVideoMemory((uint8*)m_pFrameBuffer, divxFileInfo.width, divxFileInfo.height, eTF_0888, eTF_0888, 0, 0, FILTER_LINEAR, 0, "$VideoPanel", FT_DYNAMIC);
  else
    pPanel->m_iTextureID = 	pPanel->GetUISystem()->GetIRenderer()->DownLoadToVideoMemory((uint8*)m_pFrameBuffer, divxFileInfo.width, divxFileInfo.height, eTF_0888, eTF_0888, 0, 0, FILTER_LINEAR, 0, NULL, FT_DYNAMIC);

	if (pPanel->m_iTextureID == -1)
	{
		assert(!"Failed to create video memory surface for blitting video!");

		delete[] pPanel->m_pSwapBuffer;
		pPanel->m_pSwapBuffer = 0;

		//BinkClose(m_hBink);
		//m_hBink = 0;
		
		Stop_DivX();
		pPanel->OnError("");

		return 0;
	}

	m_LastFramer			=0xffffffff;
	return 1;
}



//-----------------------------------------------------------------------------
// Name: UpdateVideo()
// Desc: Handles video-playback
//-----------------------------------------------------------------------------
bool CDivXPlayer::Update_DivX( CUIVideoPanel* pPanel )
{

	if (divxFile && pPanel->m_bPlaying)
	{

		f64 frequency	= GetQPF();
		static f64 OldTime		= GetQPC()/frequency;

//---------------------------------------------------

		f64 sec		= GetQPC()/frequency;
		f64 delta	= sec-OldTime;
		OldTime		= GetQPC()/frequency;
		m_VideoTime+=(f32)delta;

		if (m_FrameCounter==0) {
			OldTime	=	GetQPC()/frequency;
			m_VideoTime=0.0f;
		}

		bool EndOfFile=1;

		uint32 FrameNo=(uint32)(m_VideoTime*divxFileInfo.videoFrameRate);

		if (FrameNo<divxFileInfo.numVideoFrames) {

			EndOfFile=0;

			if ( m_LastFramer!=FrameNo ) 
			{

				retVal = DivxGotoFrame(divxFile, FrameNo );
				assert(retVal==0);
				DivxDoFrame(divxFile);

				/*
				uint32 w=divxFileInfo.width;
				uint32 h=divxFileInfo.height;
				uint64* fb1=(uint64*)m_pFrameBuffer;
				uint64* fb2=fb1+h*w/2;
				for(uint32 y=h; y>0; y--) {
					u64* fbuf=fb1+((y-1)*w/2);
					for(uint32 x=0; x<w/2; x++) {	fb2[x]=fbuf[x];	}
					fb2+=w/2;
				}*/

				if (pPanel->m_iTextureID!=-1)	
					pPanel->GetUISystem()->GetIRenderer()->UpdateTextureInVideoMemory(pPanel->m_iTextureID, m_pFrameBuffer, 0, 0, divxFileInfo.width, divxFileInfo.height, eTF_8888);
					//pPanel->GetUISystem()->GetIRenderer()->UpdateTextureInVideoMemory(pPanel->m_iTextureID, (uint8*)(fb1+h*w/2), 0, 0, w, h, eTF_8888);

				//uint32* ptr=(uint32*)m_pFrameBuffer;
				//for(uint32 x=0; x<(divxFileInfo.width*divxFileInfo.height); x++)	ptr[x]=0;

				if (m_FrameCounter==0) {
					if (divxFileInfo.audio_bytes) { PlaySound();	}
					else { StopSound(); }
				}
				m_FrameCounter++;
				m_LastFramer=FrameNo;
			}

		}

//-------------------------------------------------------------------

		if (EndOfFile) {
			StopSound();
			m_LastFramer=0xffffffff;
			pPanel->m_bPaused = 0;
			pPanel->m_bPlaying = 0;
			pPanel->OnFinished();
		}
	}

	return 1;
}



//-----------------------------------------------------------------------------
// Name: Stop_DivX()
// Desc: stops video and releases all resources 
//-----------------------------------------------------------------------------
void CDivXPlayer::Stop_DivX() {

	m_AudioCounter=0;
	m_FrameCounter=0;
	m_VideoTime=0.0f;

	if (divxFile) {

		if (divxFileInfo.audio_bytes) { 
			StopSound(); 
			if (m_pAudioBuffer) delete m_pAudioBuffer;
			m_pAudioBuffer=0;
			g_DivXPlayer.m_StreamingSound.Release();
			g_DivXPlayer.m_StreamingSound.m_SoundEnabled=0;
			SAFE_RELEASE( g_DivXPlayer.m_SoundManager.m_pDS );
			CloseHandle( m_hNotificationEvent	);
		}

		CloseDivxFile(divxFile);
		divxFile=0;

	} else {

		//setup the buffers
		uint32 q0=MAX_NUM_VID_BUFFERS;
		uint32 q1=MAX_NUM_AUD_BUFFERS;
		uint32 q2=MAXSBUF;
		int retVal = DivxSetNumAudVidBuffers( MAX_NUM_VID_BUFFERS, MAX_NUM_AUD_BUFFERS, q2 );
		assert(!retVal);
	}
	
	if (m_pFrameBuffer) delete m_pFrameBuffer;
	m_pFrameBuffer=0;

}


/*
void CDivXPlayer::Release_DivX() {

}*/











//-----------------------------------------------------------------------------
// Name: PlayBuffer()
// Desc: Reset the buffer, fill it with sound, and starting it playing
//-----------------------------------------------------------------------------
HRESULT CDivXPlayer::PlaySound( )
{

		BOOL bLooped=0;
    
		HRESULT hr;
    
    if( NULL == g_DivXPlayer.m_StreamingSound.m_SoundEnabled )
        return E_FAIL; // Sanity check

    hr = g_DivXPlayer.m_StreamingSound.Reset();
		assert(hr==S_OK);

    // Fill the entire buffer with wave data, and if the wav file is small then
    // repeat the wav file if the user wants to loop the file, otherwise fill in silence 
    LPDIRECTSOUNDBUFFER pDSB = g_DivXPlayer.m_StreamingSound.GetBuffer( 0 );

    hr = g_DivXPlayer.m_StreamingSound.FillBufferWithSound( pDSB, bLooped );
		assert(hr==S_OK);

    // Always play with the LOOPING flag since the streaming buffer
    // wraps around before the entire WAV is played
    if( FAILED( hr = g_DivXPlayer.m_StreamingSound.Play( 0, DSBPLAY_LOOPING ) ) )
		assert(hr==S_OK);

		return S_OK;
}



//-----------------------------------------------------------------------------
// Name: PlayBuffer()
// Desc: Reset the buffer, fill it with sound, and starting it playing
//-----------------------------------------------------------------------------
HRESULT CDivXPlayer::StopSound( )
{
	HRESULT hr;

	if( NULL == g_DivXPlayer.m_StreamingSound.m_SoundEnabled )	return E_FAIL; // Sanity check

	hr = g_DivXPlayer.m_StreamingSound.Stop();
	assert(hr==S_OK);
	hr = g_DivXPlayer.m_StreamingSound.Reset();
	assert(hr==S_OK);

	return S_OK;
}




















//-----------------------------------------------------------------------------
// Name: CSoundManager::Initialize()
// Desc: Initializes the IDirectSound object and also sets the primary buffer
//       format.  This function must be called before any others.
//-----------------------------------------------------------------------------
HRESULT CSoundManager::Initialize( HWND  hWnd, DWORD dwCoopLevel )
{
    HRESULT             hr;

    SAFE_RELEASE( m_pDS );

	HRESULT	hr00=DSERR_ALLOCATED;  
	HRESULT	hr01=DSERR_INVALIDPARAM;  
	HRESULT	hr02=DSERR_NOAGGREGATION;  
	HRESULT	hr03=DSERR_NODRIVER;  
	HRESULT	hr04=DSERR_OUTOFMEMORY;

    // Create IDirectSound using the primary sound device
    hr = DirectSoundCreate8( NULL, &m_pDS, NULL );
	if (hr) return hr;
	assert(hr==S_OK);

    // Set DirectSound coop level 
    hr = m_pDS->SetCooperativeLevel( hWnd, dwCoopLevel );
	if (hr) return hr;
	assert(hr==S_OK);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSoundManager::SetPrimaryBufferFormat()
// Desc: Set primary buffer to a specified format 
//       !WARNING! - Setting the primary buffer format and then using this
//                   same dsound object for DirectMusic messes up DirectMusic! 
//       For example, to set the primary buffer format to 22kHz stereo, 16-bit
//       then:   dwPrimaryChannels = 2
//               dwPrimaryFreq     = 22050, 
//               dwPrimaryBitRate  = 16
//-----------------------------------------------------------------------------
HRESULT CSoundManager::SetPrimaryBufferFormat( DWORD dwPrimaryChannels, 
                                               DWORD dwPrimaryFreq, 
                                               DWORD dwPrimaryBitRate )
{
    HRESULT             hr;
    LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

    // Get the primary buffer 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize        = sizeof(DSBUFFERDESC);
    dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0;
    dsbd.lpwfxFormat   = NULL;
       
    hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL );
		assert(hr==S_OK);

    WAVEFORMATEX wfx;
    ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM; 
    wfx.nChannels       = (WORD) dwPrimaryChannels; 
    wfx.nSamplesPerSec  = (DWORD) dwPrimaryFreq; 
    wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
    wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
    wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

    hr = pDSBPrimary->SetFormat(&wfx);
		assert(hr==S_OK);

    SAFE_RELEASE( pDSBPrimary );

    return S_OK;
}











//-----------------------------------------------------------------------------
// Name: CSoundManager::CreateStreaming()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CSoundManager::CreateStreaming( 
                                        DWORD dwCreationFlags, 
                                        GUID guid3DAlgorithm,
                                        DWORD dwNotifyCount, 
                                        DWORD dwNotifySize, 
                                        HANDLE hNotifyEvent )
{
    HRESULT hr;

    if( m_pDS == NULL )
        return CO_E_NOTINITIALIZED;

		if(  hNotifyEvent == NULL )
        return E_INVALIDARG;

    LPDIRECTSOUNDBUFFER pDSBuffer      = NULL;
    DWORD               dwDSBufferSize = NULL;
    DSBPOSITIONNOTIFY*  aPosNotify     = NULL; 
    LPDIRECTSOUNDNOTIFY pDSNotify      = NULL;

		uint32 frequency=44100/1;

		m_WaveFile.m_wfx.wFormatTag			=1;
		m_WaveFile.m_wfx.nChannels			=2;
		m_WaveFile.m_wfx.nSamplesPerSec	=frequency;
		m_WaveFile.m_wfx.nAvgBytesPerSec=m_WaveFile.m_wfx.nSamplesPerSec*4;
		m_WaveFile.m_wfx.nBlockAlign		=0x04;
		m_WaveFile.m_wfx.wBitsPerSample	=0x10;
		m_WaveFile.m_wfx.cbSize					=0;


		//pWaveFile->m_hmmio->unused=0x10000;

		m_WaveFile.m_ck.ckid				=0;// 0x61746164;
		m_WaveFile.m_ck.cksize			=0; //0x0077a100;
		m_WaveFile.m_ck.dwDataOffset=0;  //0x2c;
		m_WaveFile.m_ck.dwFlags			=0;
		m_WaveFile.m_ck.fccType			=0;

		m_WaveFile.m_ckRiff.ckid					=0;//0x46464952;
		m_WaveFile.m_ckRiff.cksize				=0;//0x0077a124;
		m_WaveFile.m_ckRiff.fccType				=0;//0x45564157;
		m_WaveFile.m_ckRiff.dwDataOffset	=0;//0x00000008;
		m_WaveFile.m_ckRiff.dwFlags				=0;//0x00000000;

		m_WaveFile.m_dwSize=0;//0x77a100;

    // Figure out how big the DSound buffer should be 
    dwDSBufferSize = dwNotifySize * dwNotifyCount;

    // Set up the direct sound buffer.  Request the NOTIFY flag, so
    // that we are notified as the sound buffer plays.  Note, that using this flag
    // may limit the amount of hardware acceleration that can occur. 
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = dwCreationFlags | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
    dsbd.dwBufferBytes   = dwDSBufferSize;
    dsbd.guid3DAlgorithm = guid3DAlgorithm;
    dsbd.lpwfxFormat     = &m_WaveFile.m_wfx;

    hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBuffer, NULL );
		assert(hr==S_OK);

    // Create the notification events, so that we know when to fill
    // the buffer as the sound plays. 
    hr = pDSBuffer->QueryInterface( IID_IDirectSoundNotify, (VOID**)&pDSNotify );
		assert(hr==S_OK);

    aPosNotify = new DSBPOSITIONNOTIFY[ dwNotifyCount ];
    if( aPosNotify == NULL )
        return E_OUTOFMEMORY;

    for( DWORD i = 0; i < dwNotifyCount; i++ )
    {
        aPosNotify[i].dwOffset     = (dwNotifySize * i) + dwNotifySize - 1;
        aPosNotify[i].hEventNotify = hNotifyEvent;             
    }
    
    // Tell DirectSound when to notify us. The notification will come in the from 
    // of signaled events that are handled in WinMain()
    hr = pDSNotify->SetNotificationPositions( dwNotifyCount, aPosNotify );
		assert(hr==S_OK);

    SAFE_RELEASE( pDSNotify );
    SAFE_DELETE_ARRAY( aPosNotify );

    // Create the sound
    g_DivXPlayer.m_StreamingSound.Init( pDSBuffer, dwDSBufferSize, &m_WaveFile, dwNotifySize );
		g_DivXPlayer.m_StreamingSound.m_SoundEnabled=1;
    return S_OK;
}








//-----------------------------------------------------------------------------
// Name: CSound::FillBufferWithSound()
// Desc: Fills a DirectSound buffer with a sound file 
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::FillBufferWithSound( LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger )
{
    HRESULT hr; 
    VOID*   pDSLockedBuffer      = NULL; // Pointer to locked buffer memory
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file 

    if( pDSB == NULL )
        return CO_E_NOTINITIALIZED;

    // Make sure we have focus, and we didn't just switch in from
    // an app which had a DirectSound device
   // hr = RestoreBuffer( pDSB, NULL ); 
		//assert(hr==S_OK);

		hr = RestoreBuffer( pDSB, NULL ); 

    // Lock the buffer down
    hr = pDSB->Lock( 0, m_dwDSBufferSize,&pDSLockedBuffer, &dwDSLockedBufferSize, NULL, NULL, 0L );
		assert(hr==S_OK);
		assert(dwDSLockedBufferSize==0x00081330);
    // Reset the wave file to the beginning 
    //m_pWaveFile->ResetFile();


		//11111111111111111
		int retVal;

		uint32 a0=DIVX_OK;	//The function was successful.
		uint32 a1=DIVX_FAIL;//The function failed.
		uint32 a2=INVALID_HANDLE;	//Retuned when handle value is invalid.
		int BytesReturned;
		retVal = 	DivxNextAudioChunk(g_DivXPlayer.divxFile,&BytesReturned);
		assert(!retVal);
//		assert (BytesReturned==MAXSBUF);


		g_DivXPlayer.m_AudioCounter=0;
		uint8* p=g_DivXPlayer.m_pAudioBuffer;
		uint8* sbuf=(uint8*)pDSLockedBuffer;
		uint32 t=0;
		for (uint32 x=g_DivXPlayer.m_AudioCounter; x<(g_DivXPlayer.m_AudioCounter+dwDSLockedBufferSize); x++) {
			sbuf[t]=p[x]; t++;
		}
		g_DivXPlayer.m_AudioCounter+=dwDSLockedBufferSize;
		dwWavDataRead=dwDSLockedBufferSize;
		

    if( dwWavDataRead == 0 )
    {
        // Wav is blank, so just fill with silence
        FillMemory( (BYTE*) pDSLockedBuffer,dwDSLockedBufferSize, (BYTE)(g_DivXPlayer.m_SoundManager.m_WaveFile.m_wfx.wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else if( dwWavDataRead < dwDSLockedBufferSize )
    {
        // If the wav file was smaller than the DirectSound buffer, 
        // we need to fill the remainder of the buffer with data 
        if( bRepeatWavIfBufferLarger )
        {       
            // Reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwWavDataRead;    // From previous call above.
            while( dwReadSoFar < dwDSLockedBufferSize )
            {  
								assert(0);
								// This will keep reading in until the buffer is full for very short files
                //hr = m_pWaveFile->ResetFile();
								//assert(hr==S_OK);

              //  hr = m_pWaveFile->Read( (BYTE*)pDSLockedBuffer + dwReadSoFar, dwDSLockedBufferSize - dwReadSoFar,&dwWavDataRead );
							//	assert(hr==S_OK);

                dwReadSoFar += dwWavDataRead;
            } 
        }
        else
        {
            // Don't repeat the wav file, just fill in silence 
            FillMemory( (BYTE*) pDSLockedBuffer + dwWavDataRead, dwDSLockedBufferSize - dwWavDataRead, (BYTE)(g_DivXPlayer.m_SoundManager.m_WaveFile.m_wfx.wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }

    // Unlock the buffer, we don't need it anymore.
    pDSB->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CSound::RestoreBuffer()
// Desc: Restores the lost buffer. *pbWasRestored returns TRUE if the buffer was 
//       restored.  It can also NULL if the information is not needed.
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::RestoreBuffer( LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored )
{
    HRESULT hr;

    if( pDSB == NULL )
        return CO_E_NOTINITIALIZED;

		if( pbWasRestored )
        *pbWasRestored = FALSE;

    DWORD dwStatus;
		hr = pDSB->GetStatus( &dwStatus );
		assert(hr==S_OK);

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
        do 
        {
            hr = pDSB->Restore();
#if !defined(LINUX) && !defined(NOT_USE_DIVX_SDK)
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
#endif
        }
        while( ( hr = pDSB->Restore() ) == DSERR_BUFFERLOST );

        if( pbWasRestored != NULL )
            *pbWasRestored = TRUE;

        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: CSound::GetFreeBuffer()
// Desc: Finding the first buffer that is not playing and return a pointer to 
//       it, or if all are playing return a pointer to a randomly selected buffer.
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CStreamingSound::GetFreeBuffer()
{
    if( m_apDSBuffer == NULL )
        return FALSE; 

    for( DWORD i=0; i<m_dwNumBuffers; i++ )
    {
        if( m_apDSBuffer[i] )
        {  
            DWORD dwStatus = 0;
            m_apDSBuffer[i]->GetStatus( &dwStatus );
            if ( ( dwStatus & DSBSTATUS_PLAYING ) == 0 )
                break;
        }
    }

    if( i != m_dwNumBuffers )
        return m_apDSBuffer[ i ];
    else
        return m_apDSBuffer[ rand() % m_dwNumBuffers ];
}




//-----------------------------------------------------------------------------
// Name: CSound::GetBuffer()
// Desc: 
//-----------------------------------------------------------------------------
LPDIRECTSOUNDBUFFER CStreamingSound::GetBuffer( DWORD dwIndex )
{
    if( m_apDSBuffer == NULL )
        return NULL;
    if( dwIndex >= m_dwNumBuffers )
        return NULL;

    return m_apDSBuffer[dwIndex];
}





//-----------------------------------------------------------------------------
// Name: CSound::Play()
// Desc: Plays the sound using voice management flags.  Pass in DSBPLAY_LOOPING
//       in the dwFlags to loop the sound
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::Play( DWORD dwPriority, DWORD dwFlags, LONG lVolume, LONG lFrequency, LONG lPan )
{
    HRESULT hr;
    BOOL    bRestored;

    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    LPDIRECTSOUNDBUFFER pDSB = GetFreeBuffer();
		assert(pDSB);

    // Restore the buffer if it was lost
		hr = RestoreBuffer( pDSB, &bRestored );
	//	assert(hr==S_OK);

    if( bRestored )
    {
        // The buffer was restored, so we need to fill it with new data
        hr = FillBufferWithSound( pDSB, FALSE );
				assert(hr==S_OK);
    }

    if( m_dwCreationFlags & DSBCAPS_CTRLVOLUME )
    {
        pDSB->SetVolume( lVolume );
    }

    if( lFrequency != -1 && 
        (m_dwCreationFlags & DSBCAPS_CTRLFREQUENCY) )
    {
        pDSB->SetFrequency( lFrequency );
    }
    
    if( m_dwCreationFlags & DSBCAPS_CTRLPAN )
    {
        pDSB->SetPan( lPan );
    }
    
    return pDSB->Play( 0, dwPriority, dwFlags );
}





//-----------------------------------------------------------------------------
// Name: CSound::Stop()
// Desc: Stops the sound from playing
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::Stop()
{
    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    HRESULT hr = 0;

    for( DWORD i=0; i<m_dwNumBuffers; i++ )
        hr |= m_apDSBuffer[i]->Stop();

    return hr;
}









//-----------------------------------------------------------------------------
// Name: CStreamingSound::HandleWaveStreamNotification()
// Desc: Handle the notification that tells us to put more wav data in the 
//       circular buffer
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::HandleWaveStreamNotification( BOOL bLoopedPlay )
{
    HRESULT hr;
    DWORD   dwCurrentPlayPos;
    DWORD   dwPlayDelta;
    DWORD   dwBytesWrittenToBuffer;
    VOID*   pDSLockedBuffer = NULL;
    VOID*   pDSLockedBuffer2 = NULL;

		DWORD   dwDSLockedBufferSize;
    DWORD   dwDSLockedBufferSize2;

    if( m_apDSBuffer == NULL )
        return CO_E_NOTINITIALIZED;

    // Restore the buffer if it was lost
    BOOL bRestored;
		hr = RestoreBuffer( m_apDSBuffer[0], &bRestored );
//		assert(hr==S_OK);

    if( bRestored )
    {
        // The buffer was restored, so we need to fill it with new data
        hr = FillBufferWithSound( m_apDSBuffer[0], FALSE );
				assert(hr==S_OK);
        return S_OK;
    }

    // Lock the DirectSound buffer
    hr = m_apDSBuffer[0]->Lock( m_dwNextWriteOffset, m_dwNotifySize,  &pDSLockedBuffer, &dwDSLockedBufferSize, &pDSLockedBuffer2, &dwDSLockedBufferSize2, 0L );
		assert(hr==S_OK);
		assert(dwDSLockedBufferSize==0x000204cc);

    // m_dwDSBufferSize and m_dwNextWriteOffset are both multiples of m_dwNotifySize, 
    // it should the second buffer, so it should never be valid
    if( pDSLockedBuffer2 != NULL )
        return E_UNEXPECTED; 

    if( !m_bFillNextNotificationWithSilence )
    {

			uint32 BufferEnd = (g_DivXPlayer.m_AudioCounter&(MAXSBUF-1))+dwDSLockedBufferSize;

			if (BufferEnd>MAXSBUF){
				uint32 ToBig	=	BufferEnd-MAXSBUF;
				uint32 BytesLeft	=	dwDSLockedBufferSize-ToBig;
				
				uint8* p=g_DivXPlayer.m_pAudioBuffer;
				uint8* sbuf=(uint8*)pDSLockedBuffer;
				uint32 t=0;
				for (uint32 x=(g_DivXPlayer.m_AudioCounter&(MAXSBUF-1)); x<((g_DivXPlayer.m_AudioCounter&(MAXSBUF-1))+BytesLeft); x++) {
					sbuf[t]=p[x]; t++;
				}
				g_DivXPlayer.m_AudioCounter+=BytesLeft;

				int BytesReturned;
				DivxNextAudioChunk(g_DivXPlayer.divxFile,&BytesReturned);

				for (uint32 x=(g_DivXPlayer.m_AudioCounter&(MAXSBUF-1)); x<((g_DivXPlayer.m_AudioCounter&(MAXSBUF-1))+ToBig); x++) {
					sbuf[t]=p[x]; t++;
				}

				g_DivXPlayer.m_AudioCounter+=ToBig;

				dwBytesWrittenToBuffer=dwDSLockedBufferSize;
					
			} else {

				//streaming
				hr = 0;
				uint8* p=g_DivXPlayer.m_pAudioBuffer;
				uint8* sbuf=(uint8*)pDSLockedBuffer;
				uint32 t=0;
				for (uint32 x=(g_DivXPlayer.m_AudioCounter&(MAXSBUF-1)); x<((g_DivXPlayer.m_AudioCounter&(MAXSBUF-1))+dwDSLockedBufferSize); x++) {
					sbuf[t]=p[x]; t++;
				}
				g_DivXPlayer.m_AudioCounter+=dwDSLockedBufferSize;
				dwBytesWrittenToBuffer=dwDSLockedBufferSize;
			}

			//if( FAILED(hr) )           
           // return DXTRACE_ERR( TEXT("Read"), hr );
    }
    else
    {
        // Fill the DirectSound buffer with silence
        FillMemory( pDSLockedBuffer, dwDSLockedBufferSize, (BYTE)( g_DivXPlayer.m_SoundManager.m_WaveFile.m_wfx.wBitsPerSample == 8 ? 128 : 0 ) );
        dwBytesWrittenToBuffer = dwDSLockedBufferSize;
    }

    // If the number of bytes written is less than the 
    // amount we requested, we have a short file.
    if( dwBytesWrittenToBuffer < dwDSLockedBufferSize )
    {
        if( !bLoopedPlay ) 
        {
            // Fill in silence for the rest of the buffer.
            FillMemory( (BYTE*) pDSLockedBuffer + dwBytesWrittenToBuffer, 
                        dwDSLockedBufferSize - dwBytesWrittenToBuffer, 
                        (BYTE)(g_DivXPlayer.m_SoundManager.m_WaveFile.m_wfx.wBitsPerSample == 8 ? 128 : 0 ) );

            // Any future notifications should just fill the buffer with silence
            m_bFillNextNotificationWithSilence = TRUE;
        }
        else
        {
            // We are looping, so reset the file and fill the buffer with wav data
            DWORD dwReadSoFar = dwBytesWrittenToBuffer;    // From previous call above.
            while( dwReadSoFar < dwDSLockedBufferSize )
            {  
                // This will keep reading in until the buffer is full (for very short files).
								assert(0);		
								//hr = m_pWaveFile->ResetFile();
								//if( FAILED(hr) )
                  //  return DXTRACE_ERR( TEXT("ResetFile"), hr );

								assert(0);		

                dwReadSoFar += dwBytesWrittenToBuffer;
            } 
        } 
    }

    // Unlock the DirectSound buffer
    m_apDSBuffer[0]->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );

    // Figure out how much data has been played so far.  When we have played
    // past the end of the file, we will either need to start filling the
    // buffer with silence or starting reading from the beginning of the file, 
    // depending if the user wants to loop the sound
    hr = m_apDSBuffer[0]->GetCurrentPosition( &dwCurrentPlayPos, NULL );
		assert(hr==S_OK);

    // Check to see if the position counter looped
    if( dwCurrentPlayPos < m_dwLastPlayPos )
        dwPlayDelta = ( m_dwDSBufferSize - m_dwLastPlayPos ) + dwCurrentPlayPos;
    else
        dwPlayDelta = dwCurrentPlayPos - m_dwLastPlayPos;

    m_dwPlayProgress += dwPlayDelta;
    m_dwLastPlayPos = dwCurrentPlayPos;

    // If we are now filling the buffer with silence, then we have found the end so 
    // check to see if the entire sound has played, if it has then stop the buffer.
    if( m_bFillNextNotificationWithSilence )
    {
        // We don't want to cut off the sound before it's done playing.
        if( m_dwPlayProgress >= g_DivXPlayer.m_SoundManager.m_WaveFile.GetSize() )
        {
            m_apDSBuffer[0]->Stop();
        }
    }

    // Update where the buffer will lock (for next time)
    m_dwNextWriteOffset += dwDSLockedBufferSize; 
    m_dwNextWriteOffset %= m_dwDSBufferSize; // Circular buffer

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: CStreamingSound::Reset()
// Desc: Resets the sound so it will begin playing at the beginning
//-----------------------------------------------------------------------------
HRESULT CStreamingSound::Reset()
{
    HRESULT hr;

    if( m_apDSBuffer[0] == NULL )
        return CO_E_NOTINITIALIZED;

    m_dwLastPlayPos     = 0;
    m_dwPlayProgress    = 0;
    m_dwNextWriteOffset = 0;
    m_bFillNextNotificationWithSilence = FALSE;

    // Restore the buffer if it was lost
    BOOL bRestored;
		hr = RestoreBuffer( m_apDSBuffer[0], &bRestored );
//		assert(hr==S_OK);

    if( bRestored )
    {
        // The buffer was restored, so we need to fill it with new data
        hr = FillBufferWithSound( m_apDSBuffer[0], FALSE );
				assert(hr==S_OK);
    }

    return m_apDSBuffer[0]->SetCurrentPosition( 0L );  
}




//-----------------------------------------------------------------------------
// Name: NotificationProc()
// Desc: Handles dsound notifcation events
//-----------------------------------------------------------------------------
DWORD WINAPI NotificationProc( LPVOID lpParameter )
{
    HRESULT hr;
    HWND    hDlg = (HWND) lpParameter;
    MSG     msg;
    DWORD   dwResult;
    BOOL    bDone = FALSE;
    BOOL    bLooped;

    while( !bDone ) 
    { 
        dwResult = MsgWaitForMultipleObjects( 1, &g_DivXPlayer.m_hNotificationEvent,FALSE, INFINITE, QS_ALLEVENTS );

				switch( dwResult )
        {
            case WAIT_OBJECT_0 + 0:
                // g_hNotificationEvent is signaled

                // This means that DirectSound just finished playing 
                // a piece of the buffer, so we need to fill the circular 
                // buffer with new sound from the wav file
                bLooped = 0;//( IsDlgButtonChecked( hDlg, IDC_LOOP_CHECK ) == BST_CHECKED );

								hr = g_DivXPlayer.m_StreamingSound.HandleWaveStreamNotification( bLooped );
								assert(hr==S_OK);
                break;

            case WAIT_OBJECT_0 + 1:
                // Messages are available
                while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
                { 
                    if( msg.message == WM_QUIT )
                        bDone = TRUE;
                }
                break;
        }
    }

    return 0;
}

#endif

