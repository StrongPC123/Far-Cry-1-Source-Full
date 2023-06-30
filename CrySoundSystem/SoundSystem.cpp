//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2001
//
//  CrySound Source Code
//
//  File: SoundSystem.cpp
//  Description: ISoundSystem interface implementation.
//
//  History:
//	-June 06,2001:Implemented by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#ifndef _XBOX
#include <ISystem.h>
#include <CrySizer.h>
#include <algorithm>
#include <IConsole.h>
#include <ITimer.h>
#include <Cry_Math.h>
#include <Cry_Camera.h>
#include <IRenderer.h>
#include "SoundSystem.h"
#include "MusicSystem.h"
#include "Sound.h"
#include <I3dEngine.h> //needed to check if the listener is in indoor or outdoor
#include <ICryPak.h> //needed to check if the listener is in indoor or outdoor

//#define CS_BUFFERSIZE   10         /* millisecond value for FMOD buffersize. */

#ifdef WIN64
//#define CS_SAFEBORDER 16
#endif

//////////////////////////////////////////////////////////////////////////
void* _DSPUnit_SFXFilter_Callback(void *pOriginalBuffer, void *pNewBuffer, int nLength, INT_PTR nParam)
{
	if (!nParam)
		return pNewBuffer;
	CSoundSystem *pOwner=(CSoundSystem*)nParam;
	return pOwner->DSPUnit_SFXFilter_Callback(pOriginalBuffer, pNewBuffer, nLength);
}

extern "C"
{

#if CS_SAFEBORDER
	enum {g_cFillConst = 0xCE};
void CheckSafeBorder (char* p)
{
	for (unsigned i = 0; i < CS_SAFEBORDER; ++i)
		if (p[i] != (char)(g_cFillConst+i))
			__debugbreak();
}

void FillSafeBofder (char* p)
{
	for (unsigned i = 0; i < CS_SAFEBORDER; ++i)
		p[i] = (char)(g_cFillConst+i);
}
#endif
static void * F_CALLBACKAPI CrySound_Alloc (unsigned int size)
{
#if CS_SAFEBORDER
	unsigned* pN = (unsigned*)malloc (size + 2 * CS_SAFEBORDER + sizeof(unsigned));
	*pN = size;
	char* p = (char*)(pN+1);
	FillSafeBofder(p);
	FillSafeBofder(p + size + CS_SAFEBORDER);
	return p + CS_SAFEBORDER;
#else
	return malloc (size);
#endif
}

static void F_CALLBACKAPI CrySound_Free (void* ptr)
{
#if CS_SAFEBORDER
	char* pOld = ((char*)ptr) - CS_SAFEBORDER;
	unsigned* pNOld = ((unsigned*)pOld) - 1;

	CheckSafeBorder(pOld);
	CheckSafeBorder(pOld + *pNOld + CS_SAFEBORDER);

	free (pNOld);
#else
	free (ptr);
#endif
}

static void * F_CALLBACKAPI CrySound_Realloc (void *ptr, unsigned int size)
{
#if CS_SAFEBORDER
	char* pOld = ((char*)ptr) - CS_SAFEBORDER;
	unsigned* pNOld = ((unsigned*)pOld) - 1;

	char* pRet = (char*)CrySound_Alloc(size);
	memcpy (pRet, ptr, min(*pNOld,size));
	CrySound_Free (ptr);
	return pRet;
#else
	return realloc (ptr, size);
#endif
}

}

//////////////////////////////////////////////////////////////////////////
// File callbacks for fmod.
//////////////////////////////////////////////////////////////////////////

#ifdef CS_VERSION_372

static void * F_CALLBACKAPI CrySound_fopen( const char *name )
{
	// File is opened for streaming.
	FILE *file = GetISystem()->GetIPak()->FOpen( name,"rb",ICryPak::FOPEN_HINT_DIRECT_OPERATION );
	//FILE *file = GetISystem()->GetIPak()->FOpen( name,"rb" );
	if (!file)
	{
		GetISystem()->Warning( VALIDATOR_MODULE_SOUNDSYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,"Sound %s failed to load", name );
	}
	return (void *)file;
}

static void F_CALLBACKAPI CrySound_fclose( void * nFile )
{
	FILE *file = (FILE*)nFile;
	GetISystem()->GetIPak()->FClose( file );
}

static int F_CALLBACKAPI CrySound_fread( void *buffer, int size, void * nFile )
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FRead( buffer,1,size,file );
}

static int F_CALLBACKAPI CrySound_fseek( void * nFile, int pos, signed char mode)
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FSeek( file,pos,mode );
}

static int F_CALLBACKAPI CrySound_ftell( void * nFile )
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FTell( file );
}


#else 
#ifdef CS_VERSION_361

static unsigned int F_CALLBACKAPI CrySound_fopen( const char *name )
{
	// File is opened for streaming.
	FILE *file = GetISystem()->GetIPak()->FOpen( name,"rb",ICryPak::FOPEN_HINT_DIRECT_OPERATION );
	//FILE *file = GetISystem()->GetIPak()->FOpen( name,"rb" );
	if (!file)
	{
		GetISystem()->Warning( VALIDATOR_MODULE_SOUNDSYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,"Sound %s failed to load", name );
	}
	return (unsigned int)file;
}

static void F_CALLBACKAPI CrySound_fclose( unsigned int nFile )
{
	FILE *file = (FILE*)nFile;
	GetISystem()->GetIPak()->FClose( file );
}

static int F_CALLBACKAPI CrySound_fread( void *buffer, int size, unsigned int nFile )
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FRead( buffer,1,size,file );
}

static int F_CALLBACKAPI CrySound_fseek( unsigned int nFile, int pos, signed char mode)
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FSeek( file,pos,mode );
}

static int F_CALLBACKAPI CrySound_ftell( unsigned int nFile )
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FTell( file );
}

#else

static UINT_PTR F_CALLBACKAPI CrySound_fopen( const char *name )
{
	// File is opened for streaming.
	FILE *file = GetISystem()->GetIPak()->FOpen( name,"rb",ICryPak::FOPEN_HINT_DIRECT_OPERATION );
	//FILE *file = GetISystem()->GetIPak()->FOpen( name,"rb" );
	if (!file)
	{
		GetISystem()->Warning( VALIDATOR_MODULE_SOUNDSYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,"Sound %s failed to load", name );
	}
	return (UINT_PTR)file;
}

static void F_CALLBACKAPI CrySound_fclose( UINT_PTR nFile )
{
	FILE *file = (FILE*)nFile;
	GetISystem()->GetIPak()->FClose( file );
}

static int F_CALLBACKAPI CrySound_fread( void *buffer, int size, UINT_PTR nFile )
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FRead( buffer,1,size,file );
}

static int F_CALLBACKAPI CrySound_fseek( UINT_PTR nFile, int pos, signed char mode)
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FSeek( file,pos,mode );
}

static int F_CALLBACKAPI CrySound_ftell( UINT_PTR nFile )
{
	FILE *file = (FILE*)nFile;
	return GetISystem()->GetIPak()->FTell( file );
}

#endif
#endif

//////////////////////////////////////////////////////////////////////
CSoundSystem::CSoundSystem(ISystem* pSystem, HWND hWnd) : CSoundSystemCommon(pSystem)
{
	GUARD_HEAP;
	m_bOK=m_bInside=false;

	if (!pSystem) 
		return;

	m_fSFXVolume=1.0f;
	m_fMusicVolume=1.0f;	
	m_pVisArea=NULL;
	
	m_nSpeakerConfig=-1; //force to set
	m_bPause=false;
	
	m_pISystem = pSystem;			
	m_pILog=pSystem->GetILog();
	m_pTimer=pSystem->GetITimer();
	m_pStreamEngine=pSystem->GetStreamEngine();

	m_itLastInactivePos=m_lstSoundSpotsInactive.end();
	m_nInactiveSoundSpotsSize=0;
	m_nBuffersLoaded=0;

	m_nMinSoundPriority = 0;

	m_pDSPUnitSFXFilter=NULL;

	m_fDirAttCone=0.0f;

	for (int i=0;i<MAX_SOUNDSCALE_GROUPS;i++) m_fSoundScale[i]=1.0f;
	memset(m_VisAreas,0,MAX_VIS_AREAS*sizeof(int));

	m_nMuteRefCnt=0;

	m_nSampleRate=m_pCVarSampleRate->GetIVal();
	
	//if (!m_pCVARSoundEnable->GetIVal())		
	//	return;

	if (m_pCVARDummySound->GetIVal())		
		return; // creates dummy sound system

	if (CS_GetVersion() != CS_VERSION)
	{
		m_pILog->Log("Music:Init:Incorrect DLL version for CRYSOUND: Need %.2f\n",CS_VERSION);
		return;
	}

#if !defined(_DEBUG)// || defined(WIN64)
	CS_SetMemorySystem(NULL,NULL,CrySound_Alloc,CrySound_Realloc,CrySound_Free);
#endif

	m_pILog->Log("------------------------------------------CRYSOUND VERSION=%f\n",CS_GetVersion());

	//configure CS's stability under windows
	//CS_SetBufferSize(CS_BUFFERSIZE);

	//init CS
	//CS_SetOutput(CS_OUTPUT_DSOUND);	
	//CS_SetDriver(0);
	//CS_SetMixer(CS_MIXER_QUALITY_AUTODETECT);

#if 0//defined(WIN64) || defined(CS_VERSION_3_63)

	//CS_DSP_SetActive(CS_DSP_GetClearUnit(), FALSE);
	//CS_DSP_SetActive(CS_DSP_GetSFXUnit(), FALSE);
	//CS_DSP_SetActive(CS_DSP_GetMusicUnit(), FALSE);
	//CS_DSP_SetActive(CS_DSP_GetClipAndCopyUnit(), FALSE);

	CS_SetOutput(CS_OUTPUT_WINMM);
	CS_SetMaxHardwareChannels(0);
#else

	// Defines the minimum number of hardware channels.
	// if less than that, the sound engine will switch to software mixing.
	CS_SetMinHardwareChannels(m_pCVarMinHWChannels->GetIVal());
	CS_SetMaxHardwareChannels(m_pCVarMaxHWChannels->GetIVal());
#endif
	CS_SetHWND(hWnd);	

	// Assign file access callbacks to fmod to our pak file system.
	CS_File_SetCallbacks( CrySound_fopen,CrySound_fclose,CrySound_fread,CrySound_fseek,CrySound_ftell );
 
	for (int i=0; i < CS_GetNumDrivers(); i++) 
	{
		m_pILog->Log("%d - %s\n", i+1, CS_GetDriverName(i));	// print driver names
		
		if (m_pCVarCapsCheck->GetIVal()>0) //caps checking is slow	
		{		
			unsigned int caps =0;
			CS_GetDriverCaps(i,&caps); 
		
			if (caps & CS_CAPS_HARDWARE)
				m_pILog->Log(" Driver supports hardware 3D sound");
			if (caps & CS_CAPS_EAX2)
				m_pILog->Log(" Driver supports EAX 2.0 reverb");
			if (caps & CS_CAPS_EAX3)
				m_pILog->Log(" Driver supports EAX 3.0 reverb");
			//if (caps & CS_CAPS_GEOMETRY_OCCLUSIONS)
			//	m_pILog->Log(" Driver supports hardware 3d geometry processing with occlusions");
			//if (caps & CS_CAPS_GEOMETRY_REFLECTIONS)
			//	m_pILog->Log(" Driver supports hardware 3d geometry processing with reflections");
			//if (caps & CS_CAPS_EAX2)
			//	m_pILog->Log(" Driver supports EAX 2.0 reverb!");
		}
	}
	//CS_SetMixer(CS_MIXER_QUALITY_FPU); 

	if (m_pCVarCompatibleMode->GetIVal()!=0)
	{
		CS_SetBufferSize(200);
	}

	if (!CS_Init(m_nSampleRate, 256, 0))
	{
		m_pILog->Log("Cannot init CRYSOUND\n");
		CS_Close();
		return;
	}
	//CS_3D_SetRolloffFactor(0.0);
	
	m_pILog->Log("CRYSOUND Driver: %s", CS_GetDriverName(CS_GetDriver()));		
	m_pILog->Log("Total number of channels available: %d\n",CS_GetMaxChannels());
	
  // WARNING!!! Wat
  //m_pILog->Log("Total number of hardware channels available: %d\n",CS_GetNumHardwareChannels());	

	SetSpeakerConfig();

	// lets create a dsp unit for the sfx-filter
	//m_pDSPUnitSFXFilter=CS_DSP_Create(_DSPUnit_SFXFilter_Callback, SFXFILTER_PRIORITY, (INT_PTR)this);
	if (m_pDSPUnitSFXFilter)
	{
		m_fDSPUnitSFXFilterCutoff=500.0f;
		m_fDSPUnitSFXFilterResonance=0.5f;
		m_fDSPUnitSFXFilterLowLVal=0.0f;
		m_fDSPUnitSFXFilterBandLVal=0.0f;
		m_fDSPUnitSFXFilterLowRVal=0.0f;
		m_fDSPUnitSFXFilterBandRVal=0.0f;
		//CS_DSP_SetActive(m_pDSPUnitSFXFilter, TRUE);
	}

	Mute(false);

	m_bOK = true;
	m_bValidPos=false;

	m_nLastEax=EAX_PRESET_OFF;	
	m_EAXIndoor.nPreset=EAX_PRESET_OFF;
	m_EAXOutdoor.nPreset=EAX_PRESET_OFF;

	strcpy(m_szEmptyName,"NONAME");

	m_bResetVolume=false;
	m_fSFXResetVolume=1.0f;

	m_pLastEAXProps=NULL;
}

//////////////////////////////////////////////////////////////////////////
void* CSoundSystem::DSPUnit_SFXFilter_Callback(void *pOriginalBuffer, void *pNewBuffer, int nLength)
{
	GUARD_HEAP;
	// originalbuffer = crysounds original mixbuffer.
// newbuffer = the buffer passed from the previous DSP unit.
// length = length in samples at this mix time.
// param = user parameter passed through in CS_DSP_Create.
//
// modify the buffer in some fashion
	int nMixer=CS_GetMixer();
	if (nMixer!=CS_MIXER_QUALITY_FPU)
		return pNewBuffer;
	float f, q, h;
	f=2.0f*(float)sin(3.1415962*m_fDSPUnitSFXFilterCutoff/(float)CS_GetOutputRate());
	q=m_fDSPUnitSFXFilterResonance;
	float fInput;
	nLength<<=1;
	for (int i=0;i<nLength;)
	{
		// left channel
		fInput=((float*)pNewBuffer)[i];
		m_fDSPUnitSFXFilterLowLVal+=f*m_fDSPUnitSFXFilterBandLVal;
		h=fInput-m_fDSPUnitSFXFilterLowLVal-(m_fDSPUnitSFXFilterBandLVal*q);
		m_fDSPUnitSFXFilterBandLVal+=f*h;
		// writing output
		((float*)pNewBuffer)[i]=m_fDSPUnitSFXFilterLowLVal;
		// right channel
		i++;
		fInput=((float*)pNewBuffer)[i];
		m_fDSPUnitSFXFilterLowRVal+=f*m_fDSPUnitSFXFilterBandRVal;
		h=fInput-m_fDSPUnitSFXFilterLowRVal-(m_fDSPUnitSFXFilterBandRVal*q);
		m_fDSPUnitSFXFilterBandRVal+=f*h;
		// writing output
		((float*)pNewBuffer)[i]=m_fDSPUnitSFXFilterLowRVal;
		i++;
	}
	return pNewBuffer;
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::SetSpeakerConfig()
{
	GUARD_HEAP;
	int nCurrSpeakerConfig=m_pCVarSpeakerConfig->GetIVal();
	if (nCurrSpeakerConfig!=m_nSpeakerConfig)
	{
		switch (nCurrSpeakerConfig)
		{
			case CSSPEAKERCONFIG_5POINT1:
				CS_SetSpeakerMode(CS_SPEAKERMODE_DOLBYDIGITAL);
				m_pILog->Log("Set speaker mode 5.1\n");
				break;
			case CSSPEAKERCONFIG_HEADPHONE:
				CS_SetSpeakerMode(CS_SPEAKERMODE_HEADPHONES);
				m_pILog->Log("Set speaker mode head phone\n");
				break;
			case CSSPEAKERCONFIG_MONO:
				CS_SetSpeakerMode(CS_SPEAKERMODE_MONO);
				m_pILog->Log("Set speaker mode mono\n");
				break;
			case CSSPEAKERCONFIG_QUAD:
				CS_SetSpeakerMode(CS_SPEAKERMODE_QUAD);
				m_pILog->Log("Set speaker mode quad\n");
				break;
			case CSSPEAKERCONFIG_STEREO:
				CS_SetSpeakerMode(CS_SPEAKERMODE_STEREO);
				m_pILog->Log("Set speaker mode stereo\n");
				break;
			case CSSPEAKERCONFIG_SURROUND:
				m_pILog->Log("Set speaker mode surround\n");
				CS_SetSpeakerMode(CS_SPEAKERMODE_SURROUND);
				break;
		}		
		m_nSpeakerConfig=nCurrSpeakerConfig;

		//Set the pan scalar to full pan separation 'cos cs_setspeakermode will reset it.
		CS_SetPanSeperation(1.0f);
	}	
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::Reset()
{
	GUARD_HEAP;
	Silence();

	// for the sake of clarity...
	m_vecSounds.clear();	
	m_lstSoundSpotsActive.clear(); 
	m_lstSoundSpotsInactive.clear();m_nInactiveSoundSpotsSize=0;
	//m_nLastInactiveListPos=0;
	m_itLastInactivePos=m_lstSoundSpotsInactive.end();	
	m_lstFadeUnderwaterSounds.clear();

	m_autoStopSounds.clear();
	m_stoppedSoundToBeDeleted.clear();
}

//////////////////////////////////////////////////////////////////////
CSoundSystem::~CSoundSystem()
{
	GUARD_HEAP;

	Reset();

	if (m_pDSPUnitSFXFilter)
		CS_DSP_Free(m_pDSPUnitSFXFilter);
	m_pDSPUnitSFXFilter=NULL;

	if (m_bOK)
		CS_Close();	

	m_bOK = false;	

	SetEaxListenerEnvironment(EAX_PRESET_OFF);
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::Release()
{
	delete this;
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::DeactivateSound( CSound *pSound )
{
	if (pSound->m_bPlaying)
		pSound->Stop();

	if (pSound->m_State == eSoundState_Active)
	{
		m_lstSoundSpotsActive.erase(pSound);
		pSound->m_State = eSoundState_Inactive;
		m_lstSoundSpotsInactive.insert(pSound);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSoundSystem::ProcessActiveSound( CSound *cs )
{
	FUNCTION_PROFILER( GetSystem(),PROFILE_SOUND );

	bool bFadeoutFinished = false;
	bool bIsSoundPotentiallyHearable = IsSoundPH(cs);
	if (!bIsSoundPotentiallyHearable)
	{
		// decrease it as the player is leaving the room or moving from outdoor to indoor or viceversa.
		bFadeoutFinished = cs->FadeOut();
	}
	float fRatio = 1.0f;
	if (cs->m_nFlags & FLAG_SOUND_RADIUS)
	{
		float fDist2=GetSquaredDistance(cs->m_RealPos,m_vPos);
		if (fDist2 > cs->m_fMaxRadius2)
		{
			// Too far.
			return false;
		}
		if (bFadeoutFinished)
		{
			return false;
		}
		if (fDist2 > cs->m_fMinRadius2)
		{					
			//calc attenuation, cal sound ratio between min & max radius
			if (!(cs->m_nFlags & FLAG_SOUND_NO_SW_ATTENUATION))
			{
				fRatio=1.0f-((fDist2-cs->m_fMinRadius2)/cs->m_fMaxRadius2);
			}
			else
				fRatio=1.0f;
		}
	}
	if (bIsSoundPotentiallyHearable)
	{
		// check if the sound became hearable because of sound occlusion
		cs->FadeIn();
	}
	// if Steve doesnt want to change frequency and pan, this can be replaced just with set volume
	cs->SetAttrib(cs->m_nMaxVolume,fRatio*cs->m_fCurrentFade*cs->m_fRatio,cs->m_nPan,1000,false);
	return true;
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::Update()
{
	GUARD_HEAP;
	FUNCTION_PROFILER( GetSystem(),PROFILE_SOUND );
	
	// check if volume has changed
	if (Ffabs(m_pCVarMusicVolume->GetFVal()-m_fMusicVolume)>0.2f)
	{
		if (m_pCVarMusicVolume->GetFVal()<0)
			m_pCVarMusicVolume->Set(0.0f);
		m_fMusicVolume=m_pCVarMusicVolume->GetFVal();
	}

	if (m_pCVarSFXVolume->GetFVal()<0)
		m_pCVarSFXVolume->Set(0.0f);

	if (m_pCVarSFXVolume->GetFVal()>1.0f)
		m_pCVarSFXVolume->Set(1.0f);

	if ((Ffabs(m_pCVarSFXVolume->GetFVal()-m_fSFXVolume)>0.2f))
	{
		m_fSFXVolume=m_pCVarSFXVolume->GetFVal();		

		// call set volume for each sound so it can rescale itself		
		for (SoundListItor It=m_vecSounds.begin();It!=m_vecSounds.end();It++)
		{ 
			CSound *cs=(*It);
			if (cs->m_nFlags & FLAG_SOUND_MUSIC)			
				continue;			
			cs->SetVolume(cs->GetVolume());
		} // It
	}

	if (m_pISystem && m_pCVarSoundInfo->GetIVal())
	{ 
		IRenderer *pRenderer=m_pISystem->GetIRenderer();
		if (pRenderer)
		{
			int nChannels = CSound::m_PlayingChannels;
			float x = 40;
			float fColor[4]={1.0f, 1.0f, 1.0f, 0.7f};
			float fColorRed[4]={1.0f, 0.0f, 0.0f, 0.7f};
			float fColorYellow[4]={1.0f, 1.0f, 0.0f, 0.7f};
			float fColorGreen[4]={0.0f, 1.0f, 0.0f, 0.7f};
			pRenderer->Draw2dLabel(x, 1, 1, fColor, false, "SoundSystem[Debug]");
			pRenderer->Draw2dLabel(x, 10, 1, fColor, false, "  Current Voices: %d (Channels: %d), CPU-Mixing-Overhead: %3.1f", GetUsedVoices(),nChannels, GetCPUUsage());
			pRenderer->Draw2dLabel(x, 20, 1, fColor, false, "  Loaded SoundBuffers: %d (%d)", m_nBuffersLoaded,m_soundBuffers.size());
			pRenderer->Draw2dLabel(x, 30, 1, fColor, false, "  SoundSpots-Active: %d, Inactive: %d", (int)m_lstSoundSpotsActive.size(), (int)m_lstSoundSpotsInactive.size());			
			if (!m_bValidPos)
			{
				pRenderer->Draw2dLabel(x, 40, 1, fColor, false, "Listener invalid!");			
			}
			else
			{			
				if (m_bInside)
					pRenderer->Draw2dLabel(x, 40, 1, fColor, false, "Listener INSIDE: Vis area=: %s",m_pVisArea->GetName());			
				else
					pRenderer->Draw2dLabel(x, 40, 1, fColor, false, "Listener OUTSIDE ");			
			}

			float ypos=50;
			if (m_pCVarSoundInfo->GetIVal()==1) 
			{			
				for (SoundListItor It=m_lstSoundSpotsActive.begin();It!=m_lstSoundSpotsActive.end();It++)
				{			
					CSound *cs=(*It);
					if (cs->IsPlayingOnChannel())
						pRenderer->Draw2dLabel(1, ypos, 1, fColorRed, false, "<Playing>");
					else if (cs->IsUsingChannel())
						pRenderer->Draw2dLabel(1, ypos, 1, fColorYellow, false, "[%d]",cs->m_nChannel );

					float *pCurColor = fColor;
					if (!cs->m_bLoop)
						pCurColor = fColorGreen;

					if (cs->m_nFlags & FLAG_SOUND_3D)
						pRenderer->Draw2dLabel(x, ypos, 1, pCurColor, false, "%s,3d,pos=%d,%d,%d,minR=%d,maxR=%d,vol=%d",cs->GetName(),(int)(cs->m_position.x),(int)(cs->m_position.y),(int)(cs->m_position.z),(int)(cs->m_fMinDist),(int)(cs->m_fMaxDist),cs->m_nPlayingVolume );
					else
						pRenderer->Draw2dLabel(x, ypos, 1, pCurColor, false, "%s,2d,volume=%d",cs->GetName(),cs->m_nPlayingVolume );

					ypos+=10;
				} //it
			}			
			else
			{
				for (SoundBufferPropsMapItor It=m_soundBuffers.begin();It!=m_soundBuffers.end();It++)
				{			
					CSoundBuffer *pBuf=It->second;
					
					if (pBuf->GetSample())
					{
						pRenderer->Draw2dLabel(x, ypos, 1, fColor, false, "%s",pBuf->GetName());
						ypos+=10;
					}
				} //it
			}


			// All playing sounds...
			{
				ypos = 10;
				x = 400;
				pRenderer->Draw2dLabel(x, ypos, 1, fColor, false, "----- Playing Sounds -----" );
				ypos += 10;
				for (SoundListItor It=m_vecSounds.begin();It!=m_vecSounds.end();It++)
				{
					CSound *cs=(*It);
					if (!cs->IsUsingChannel())
						continue;
					float *pCurColor = fColorYellow;
					if (cs->IsPlayingOnChannel())
						pCurColor = fColorRed;

					if (cs->m_nFlags & FLAG_SOUND_3D)
						pRenderer->Draw2dLabel(x, ypos, 1, pCurColor, false, "%s,3d,pos=%d,%d,%d,minR=%d,maxR=%d,vol=%d,channel=%d",cs->GetName(),(int)(cs->m_position.x),(int)(cs->m_position.y),(int)(cs->m_position.z),(int)(cs->m_fMinDist),(int)(cs->m_fMaxDist),cs->m_nPlayingVolume,cs->m_nChannel );
					else
						pRenderer->Draw2dLabel(x, ypos, 1, pCurColor, false, "%s,2d,volume=%d,channel=%d",cs->GetName(),cs->m_nPlayingVolume,cs->m_nChannel );

					ypos+=10;
				} //it
			}
		}
	}

	Vec3_tpl<float> vPos=m_cCam.GetPos();

	if (GetLengthSquared(vPos)<1)
	{
		m_bValidPos=false;
		return; // hasnt been set yet, but the update3d function has been called
	}
	
	m_bValidPos=true;

	if (m_pCVARSoundEnable->GetIVal()==0)
	{
		Silence();
		return;
	}

	SetSpeakerConfig();
	
	bool bWasInside=m_bInside;
	m_bInside=false;
	RecomputeSoundOcclusion(true,false);
	m_bInside=(m_pVisArea!=NULL);	

	/*
	if (bWasInside!=m_bInside)
	{
		if (m_bInside)
			SetEaxListenerEnvironment(m_EAXIndoor.nPreset, (m_EAXIndoor.nPreset==-1) ? &m_EAXIndoor.EAX : NULL, FLAG_SOUND_INDOOR);
		else
			SetEaxListenerEnvironment(m_EAXOutdoor.nPreset, (m_EAXOutdoor.nPreset==-1) ? &m_EAXOutdoor.EAX : NULL, FLAG_SOUND_OUTDOOR);
	}
	*/

	// check if sr has changed
	if (m_pCVarSampleRate->GetFVal()!=m_nSampleRate)
	{
		m_nSampleRate=(int)m_pCVarSampleRate->GetFVal();
	}
 
	std::pair< SoundListItor, bool > pr;

	int nActiveSize=m_lstSoundSpotsActive.size();

	if ((nActiveSize<m_pCVarMaxSoundSpots->GetIVal()) && (!m_lstSoundSpotsInactive.empty()))
	{		
		FRAME_PROFILER( "CSoundSystem::Update:Inactive",GetSystem(),PROFILE_SOUND );

		// iterate through a couple of inactive sounds to see if they became active...
		//int nInactiveSoundSpots=(int)m_vecSoundSpotsInactive.size();		
		//if (m_nLastInactiveListPos>=nInactiveSoundSpots)
			//m_nLastInactiveListPos=0;
		if (m_itLastInactivePos==m_lstSoundSpotsInactive.end())
			m_itLastInactivePos=m_lstSoundSpotsInactive.begin();
 
		// lets choose the iteration-count, so it iterates through all sounds every m_pCVarInactiveSoundIterationTimeout seconds
		int nInactiveSoundItCount=(int)cry_ceilf((m_pTimer->GetFrameTime()/m_pCVarInactiveSoundIterationTimeout->GetFVal())*(float)m_nInactiveSoundSpotsSize);	
		if (!nInactiveSoundItCount)
			nInactiveSoundItCount=1;
		if (nInactiveSoundItCount>m_nInactiveSoundSpotsSize)
			nInactiveSoundItCount=m_nInactiveSoundSpotsSize;

		/*
		if (m_pISystem && m_pCVarSoundInfo->GetIVal())
		{ 
			float fColor[4]={1.0f, 1.0f, 1.0f, 1.0f};
			IRenderer *pRenderer=m_pISystem->GetIRenderer();
			if (pRenderer)
				pRenderer->Draw2dLabel(10, 110, 1, fColor, false, "  SoundSpots-InActive size: %d, iter count=%d ", m_nInactiveSoundSpotsSize,nInactiveSoundItCount);
		}
		*/

		SoundListItor It=m_itLastInactivePos; //m_vecSoundSpotsInactive.begin()+m_nLastInactiveListPos;

		int nSoundSpotAdded=0;

		for (int i=0;i<nInactiveSoundItCount;i++)
		{ 
			CSound *cs=(*It);
			if (cs->m_bPlaying)
			{
				float fDist2=GetSquaredDistance(cs->m_RealPos,m_vPos);
				if (fDist2<=cs->m_fPreloadRadius2)
				{					

					//////////////////////////////////////////////////////////////////////////					
					// are we getting too many soundspots?					
					if ((nActiveSize+nSoundSpotAdded)>m_pCVarMaxSoundSpots->GetIVal())
					{
						break; // resume next frame
					}
					//////////////////////////////////////////////////////////////////////////

					// move sound to active list...
					//m_vecSoundSpotsActive.push_back(cs);
					m_lstSoundSpotsActive.insert(cs);
					nSoundSpotAdded++;
					It=m_lstSoundSpotsInactive.erase(It);m_nInactiveSoundSpotsSize--;
					m_itLastInactivePos=It;
					cs->m_State=eSoundState_Active;
					cs->Preload();										
				}
				else
				{
					++It;
					//m_nLastInactiveListPos++;
					++m_itLastInactivePos;
				}
			}
			else
			{
				++It;
				//m_nLastInactiveListPos++;
				++m_itLastInactivePos;
			}

			if (It==m_lstSoundSpotsInactive.end())
			{
				It=m_lstSoundSpotsInactive.begin();
				//m_nLastInactiveListPos=0;
				m_itLastInactivePos=It;
			}

		} //i

	}

	float fCurrTime = m_pTimer->GetCurrTime();
	// process the sound spheres for fading and occlusion
	//for (SoundVecItor It=m_vecSoundSpotsActive.begin();It!=m_vecSoundSpotsActive.end();)
	SoundListItor It,nextIt;
	for (It=m_lstSoundSpotsActive.begin();It!=m_lstSoundSpotsActive.end();)
	{
		FRAME_PROFILER( "CSoundSystem::Update:Active",GetSystem(),PROFILE_SOUND );

		nextIt = It; nextIt++;
		CSound *cs=(*It);
		if (cs->m_pSound->LoadFailure())
		{
			cs->m_State=eSoundState_None;
			It = m_lstSoundSpotsActive.erase(It);
			continue;
		}

		// make sure that one-shot samples reset its playing flag when they exceed their play-time and are clipped...
		if ((!cs->m_bPlaying) || (cs->IsUsingChannel() && !cs->m_bLoop && !cs->IsPlayingOnChannel()) || cs->IsPlayLengthExpired(fCurrTime))
		{
			FRAME_PROFILER( "CSoundSystem::Update:Active-Deactivate",GetSystem(),PROFILE_SOUND );
			//++It;
			// move sound to inactive list...
			//m_vecSoundSpotsInactive.push_back(cs);

			pr=m_lstSoundSpotsInactive.insert(cs);
			m_itLastInactivePos=pr.first;
			m_nInactiveSoundSpotsSize++;

			cs->m_State=eSoundState_Inactive;
			It=m_lstSoundSpotsActive.erase(It);
			if (cs->m_bPlaying)
			{
				// Stop can release sound for autostop onces.
				cs->Stop();
			}
			continue;
		}
		// check if the sound became occluded
		bool bZeroFadeCheckDist=false;
		bool bIsSoundPotentiallyHearable = IsSoundPH(cs);
		if (!bIsSoundPotentiallyHearable)
		{
			FRAME_PROFILER( "CSoundSystem::Update:Active-FadeOut",GetSystem(),PROFILE_SOUND );
			// decrease it as the player is leaving the room
			// or moving from outdoor to indoor or viceversa
			bZeroFadeCheckDist=cs->FadeOut();
			/*{
			++It;
			continue; 
			}*/
		}

		//const char *szTest=cs->GetName(); //debug				
		float fRatio=1.0f;
		if (cs->m_nFlags & FLAG_SOUND_RADIUS)
		{
			float fDist2=GetSquaredDistance(cs->m_RealPos,m_vPos);
			if (fDist2>cs->m_fMaxRadius2)
			{
				cs->FreeChannel();
				// do not change the fading here because 
				// there is a different reason why we stopped this sound
				if (fDist2>cs->m_fPreloadRadius2)
				{
					// move sound to inactive list...
					//m_vecSoundSpotsInactive.push_back(cs);
					pr=m_lstSoundSpotsInactive.insert(cs);
					m_itLastInactivePos=pr.first;
					m_nInactiveSoundSpotsSize++;

					It=m_lstSoundSpotsActive.erase(It);
					cs->m_State=eSoundState_Inactive;
				}
				else
				{
					++It;
				}
				continue; //too far
			}
			if (bZeroFadeCheckDist)
			{
				++It;
				continue;	// the sound is faded out to 0, we just do this check here because it might also be too far away in which case it will be moved to the inactive list again...
			}
			if (fDist2>cs->m_fMinRadius2)								
			{					
				//calc attenuation				 	
				//calculate sound ratio between min & max radius												

				if (cs->m_nFlags & FLAG_SOUND_NO_SW_ATTENUATION)
				{					
					//fRatio=0.5f-((fDist2-cs->m_fMinRadius2)/cs->m_fMaxRadius2);//cs->m_fMaxSoundDistance2);
					//if (fRatio<0)
					//fRatio=0;
					fRatio=1.0f;
					//m_pILog->LogToConsole("fratio=%f",fRatio);
				}
				else
					fRatio=1.0f-((fDist2-cs->m_fMinRadius2)/cs->m_fMaxRadius2);//cs->m_fMaxSoundDistance2);
				//else				
			}
		}

		if (bIsSoundPotentiallyHearable)
		{
			// check if the sound became hearable because of sound occlusion
			cs->FadeIn();
		}

		if (!cs->IsPlayingOnChannel() && cs->IsPlayingVirtual()) //loop check 
		{
			cs->Play(fRatio*cs->m_fCurrentFade, false, false);
		}

		{
			FRAME_PROFILER( "CSoundSystem::Update:Active-SetAttrib",GetSystem(),PROFILE_SOUND );

			// if Steve doesnt want to change frequency and pan, this
			// can be replaced just with set volume
			cs->SetAttrib(cs->m_nMaxVolume,fRatio*cs->m_fCurrentFade*cs->m_fRatio,cs->m_nPan,1000,false);
		}
		++It;
	}

	//////////////////////////////////////////////////////////////////////////
	// Stop expired auto stop sounds.
	//////////////////////////////////////////////////////////////////////////
	if (!m_autoStopSounds.empty())
	{
		AutoStopSounds::iterator it,next;
		for (it = m_autoStopSounds.begin(); it != m_autoStopSounds.end(); it = next)
		{
			next = it; next++;
			CSound *pSound = *it;
			if (pSound->IsPlayingOnChannel())
			{
				bool bContinueSound = ProcessActiveSound(pSound);
				if (!bContinueSound || pSound->IsPlayLengthExpired(fCurrTime))
				{
					UnregisterAutoStopSound( pSound );
					pSound->m_bAutoStop = false;
					pSound->Stop();
					continue;
				}
			}
			else if (pSound->IsUsingChannel())
			{
				UnregisterAutoStopSound( pSound );
				pSound->m_bAutoStop = false;
				pSound->Stop();
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////

	if (m_nLastEax!=EAX_PRESET_UNDERWATER)
	{			
		// if not underwater, check if we now went to outside
		if (bWasInside!=m_bInside)
		{
			if (!m_bInside)
			{
				// if outside,set the global EAX outdoor environment
				SetEaxListenerEnvironment(m_EAXOutdoor.nPreset, (m_EAXOutdoor.nPreset==-1) ? &m_EAXOutdoor.EAX : NULL);
			}
		}
	}

	/*
	if (m_nLastEax==EAX_PRESET_UNDERWATER)
	{			
		for (It=m_lstFadeUnderwaterSounds.begin();It!=m_lstFadeUnderwaterSounds.end();)
		{			
			CSound *cs=(*It);
			SoundListItor ItNext=It;ItNext++;
			if (!cs->IsPlayingOnChannel() || cs->FadeOut())
			{
				// in case the sound has been removed already by calling stop from
				// script
				It=m_lstFadeUnderwaterSounds.find(cs); 
				if (It!=m_lstFadeUnderwaterSounds.end())
					m_lstFadeUnderwaterSounds.erase(It);
				It=ItNext;
				continue;
			}
			cs->SetAttrib(cs->m_nVolume,cs->m_fCurrentFade);
			It=ItNext;
		} //it		
	}
	*/

	// Here actually delete all auto stop sounds.
	m_stoppedSoundToBeDeleted.resize(0);

	{
		FRAME_PROFILER( "CSoundSystem::CS_Update",GetSystem(),PROFILE_SOUND );
		CS_Update();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSoundSystem::IsSoundPH(CSound *pSound)
{
	GUARD_HEAP;
	// check if indoor only or outdoor only 
	if (pSound->m_nFlags & FLAG_SOUND_OUTDOOR)
	{			
		return (!m_bInside); // fade out or fade in
	}
	
	if (pSound->m_nFlags & FLAG_SOUND_INDOOR)		
	{	
		return(m_bInside); // fade out or fade in
	}

	// at this point check if should use sound occlusion at all
	if (!(pSound->m_nFlags & FLAG_SOUND_OCCLUSION))
		return (true); // always hearable, no occlusion - proceed with fade in

	if (!m_pVisArea)
	{
		// if the listener is not inside a vis area but the sound is,
		// then the sound is not hearable 
		if (pSound->m_pArea)
			return (false); // fade out

		return (true); // both listener and sound in outdoor - fade in
	}

	// if the listener is inside a vis area but the sound is not,
	// then the sound is not hearable 
	if (!pSound->m_pArea)
		return (false); // fade out

	// now check that the area where the sound is in, isn't occluded by doors, 
	// objects or buildings	
	int numToCheck = m_pCVarVisAreaProp->GetIVal();
	for (int k=0; k < numToCheck; k++)
	{
		IVisArea *pArea=m_VisAreas[k];
		if (!pArea)
			break;
		if (pArea==pSound->m_pArea)
			return (true); // both listener and sound in indoor, and in a not-occluded area - fade in
	} //k

	return (false); //fade out
	
}

//////////////////////////////////////////////////////////////////////////
IMusicSystem* CSoundSystem::CreateMusicSystem()
{
	CMusicSystem *pMusicSystem=new CMusicSystem(m_pISystem);
	return pMusicSystem;
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::AddToSoundSystem(CSound *pSound,int dwFlags)
{
	GUARD_HEAP;
	pSound->m_nFlags=dwFlags;		
	
	bool bLoop;
	if (dwFlags & FLAG_SOUND_LOOP)
	{
		pSound->SetLoopMode(true);
		bLoop = true;
	}
	else
	{
		bLoop = false;
		pSound->SetLoopMode(false);
	}

	if (dwFlags & FLAG_SOUND_UNSCALABLE)
	{
		pSound->RemoveFromScaleGroup(SOUNDSCALE_SCALEABLE);
		pSound->RemoveFromScaleGroup(SOUNDSCALE_MISSIONHINT);
	}

	//m_vecSounds.push_back(pSound);
	m_vecSounds.insert(pSound);

	std::pair<SoundListItor,bool> pr;
	// check if the sound has a radius, and add it to a separate list
	// to speed up the checks, occlusions and sound system update
	//if (dwFlags & (FLAG_SOUND_RADIUS | FLAG_SOUND_OCCLUSION | FLAG_SOUND_INDOOR | FLAG_SOUND_OUTDOOR))	
	if (dwFlags & (FLAG_SOUND_ACTIVELIST) && bLoop)
	{
		if (m_bValidPos && (dwFlags & FLAG_SOUND_RADIUS))
		{
			float fDist2=GetSquaredDistance(pSound->m_RealPos, m_vPos);
			if (fDist2>pSound->m_fPreloadRadius2)
			{
				//m_vecSoundSpotsInactive.push_back(pSound);	// move sound to inactive list...
				pr=m_lstSoundSpotsInactive.insert(pSound);
				m_itLastInactivePos=pr.first;
				m_nInactiveSoundSpotsSize++;

				pSound->m_State=eSoundState_Inactive;
			}
		}
		if (pSound->m_State!=eSoundState_Inactive)
		{
			//m_vecSoundSpotsActive.push_back(pSound);		// move sound to active list...
			m_lstSoundSpotsActive.insert(pSound);
			pSound->m_State=eSoundState_Active;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::RegisterAutoStopSound( CSound *pSound )
{
	m_autoStopSounds.insert( pSound );
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::UnregisterAutoStopSound( CSound *pSound )
{
	m_stoppedSoundToBeDeleted.push_back( pSound );
	m_autoStopSounds.erase( pSound );
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::SetSoundActiveState(CSound *pSound, ESoundActiveState State)	// take care from where you call this function, because it may cause crashes if you change the state while the system is iterating through the lists (Update())
{
	GUARD_HEAP;
	if (pSound->m_State==State)
		return;
	
	switch (pSound->m_State)
	{
		case eSoundState_Active:
		{
			//SoundVecItor It=std::find(m_vecSoundSpotsActive.begin(), m_vecSoundSpotsActive.end(), pSound);
			SoundListItor It=m_lstSoundSpotsActive.find(pSound);
			ASSERT(It!=m_lstSoundSpotsActive.end());
			m_lstSoundSpotsActive.erase(It);
			break;
		}
		case eSoundState_Inactive:
		{
			//SoundVecItor It=std::find(m_vecSoundSpotsInactive.begin(), m_vecSoundSpotsInactive.end(), pSound);
			SoundListItor It=m_lstSoundSpotsInactive.find(pSound);
			ASSERT(It!=m_lstSoundSpotsInactive.end());
			m_itLastInactivePos=m_lstSoundSpotsInactive.erase(It);
			m_nInactiveSoundSpotsSize--;			
			break;
		}
	}
	pSound->m_State=State;
	switch (pSound->m_State)
	{
		case eSoundState_Active:
		{
			//m_vecSoundSpotsActive.push_back(pSound);
			m_lstSoundSpotsActive.insert(pSound);
			break;
		}
		case eSoundState_Inactive:
		{
			//m_vecSoundSpotsInactive.push_back(pSound);
			std::pair<SoundListItor,bool> pr;
			pr=m_lstSoundSpotsInactive.insert(pSound);
			m_itLastInactivePos=pr.first;
			m_nInactiveSoundSpotsSize++;

			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
ISound* CSoundSystem::LoadSound(const char *szFile, int flags)
{
	GUARD_HEAP;
	FUNCTION_PROFILER( GetSystem(),PROFILE_SOUND );

	CSound* pSound = NULL;
	
	if (!szFile || (!szFile[0])) 
		return (NULL);

	//if (!strlen(szFile)) 
	//	return (NULL);
 
	if (flags & FLAG_SOUND_3D)	// make all 3d sounds use the sw-attenuation
		flags|=FLAG_SOUND_RADIUS;
	
	CSoundBuffer *pSB;
	SSoundBufferProps name=SSoundBufferProps(szFile, flags);
	SoundBufferPropsMapItor nit = m_soundBuffers.find(name);
	if ((!(flags & FLAG_SOUND_STREAM)) && (nit!=m_soundBuffers.end()))	// we cannot share streams !
	{
		//sound is already present

		pSB= nit->second;

		if (flags & FLAG_SOUND_3D)
		{			
			if (pSB->GetProps().nFlags & FLAG_SOUND_2D)
			{
				m_pILog->Log("\001 [ERROR] trying to load the same sound buffer file as 2d and 3d sound (%s)",szFile);
				return (NULL);
			}
		}
		else
		if (flags & FLAG_SOUND_2D)
		{
			if (pSB->GetProps().nFlags & FLAG_SOUND_3D)
			{
				m_pILog->Log("\001 [ERROR] trying to load the same sound buffer file as 2d and 3d sound (%s)",szFile);
				return (NULL);
			}
		}

		//create a new instance	
		pSound=new CSound(this,szFile);

		pSound->m_pSound=pSB;		

		AddToSoundSystem(pSound,flags);

		return (pSound);			
	}

	// check if this is an ogg...to avoid people calling
	// loadsound and decompressing the entire ogg into memory (several megabytes)
	// but if the load sync is specified, load the sound anyway doesnt matter what
	if (!(flags & FLAG_SOUND_LOAD_SYNCHRONOUSLY))
	{
		size_t len = strlen(szFile)-1;
		const char *szExt=NULL;
		while (len)
		{
			if (szFile[len]=='.')
			{
				szExt=&szFile[len];
				break;
			}
			len--;
		}

		if (szExt && (stricmp(szExt,".ogg")==0) && (!(flags & FLAG_SOUND_STREAM)))
		{
			m_pILog->Log("\001 WARNING - THE FILE %s is a streaming sound but there is an attempt to decompress it into memory-it will not be loaded",szFile);
			return (NULL);
		}
	}

	//create a new instance	and a new sound buffer	
	pSound=new CSound(this,szFile);

	//m_pILog->LogToFile("sound %s loaded, sound size=%d",szFile,m_setSounds.size());
		
	//create a new sound buffer
	pSB=new CSoundBuffer(this, name);
	pSound->m_pSound=pSB;
	pSound->SetAttrib(255,1.0f,pSound->m_nPan);
		
	m_soundBuffers[name] = pSB;
	nit = m_soundBuffers.find(name);
	if (nit==m_soundBuffers.end())
	{
		int t=0;
	}

	AddToSoundSystem(pSound,flags);

	return (pSound);
}

//////////////////////////////////////////////////////////////////////////
void	CSoundSystem::RecomputeSoundOcclusion(bool bRecomputeListener,bool bForceRecompute,bool bReset)
{
	GUARD_HEAP;
	if (bReset)
	{
		m_pVisArea=NULL;
		return;
	}
	if (m_nMuteRefCnt>0)
		return; // don't do this during mute (usually during loading)

	FUNCTION_PROFILER( GetSystem(),PROFILE_SOUND );

	I3DEngine	*p3dEngine=m_pISystem->GetI3DEngine();
	if (p3dEngine)	
	{			
		//check where the listener is 	
		IVisArea *pCurrArea=NULL;
		if (bRecomputeListener)
		{
			Vec3_tpl<float> vPos=m_cCam.GetPos();
			pCurrArea=p3dEngine->GetVisAreaFromPos(vPos);
		}
		else
			pCurrArea=m_pVisArea;

		// avoid silly people to mess around with too many visareas
		int nMaxVis=(m_pCVarVisAreaProp->GetIVal() & (MAX_VIS_AREAS-1));

		if ((pCurrArea!=m_pVisArea) || (bForceRecompute))
		{
			// listener has changed sector or has moved from indoor to outdoor or viceversa
			m_pVisArea=pCurrArea;
			memset(m_VisAreas,0,sizeof(m_VisAreas));
			if (m_pVisArea)
			{
				// if inside let's get a list of visible areas to use for sound occlusion
				int nAreas=m_pVisArea->GetVisAreaConnections(m_VisAreas,nMaxVis,true);
				// engine doesn't return the current area - be sure to add it or
				// all the sounds currently playing in the listener area will fade out
				if (nAreas<nMaxVis)
					m_VisAreas[nAreas]=m_pVisArea;
				else
					// override the first one - this one is more important
					m_VisAreas[0]=m_pVisArea;
			}
		}			
	}
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::RemoveReference(CSound *cs)
{
	GUARD_HEAP;
	if (m_vecSounds.empty()) 
		return;
	SoundListItor It;
	//It=std::find(m_vecSounds.begin(), m_vecSounds.end(), cs);
	
	It=m_vecSounds.find(cs);

	//if (It==m_vecSounds.end())	// double release
	//	return;
	ASSERT(It!=m_vecSounds.end());
	m_vecSounds.erase(It);
	switch (cs->m_State)
	{
		case eSoundState_Active:
		{			
			//It=std::find(m_vecSoundSpotsActive.begin(), m_vecSoundSpotsActive.end(), cs);
			It=m_lstSoundSpotsActive.find(cs);
			if (It!=m_lstSoundSpotsActive.end())
				m_lstSoundSpotsActive.erase(It);
			break;
		}
		case eSoundState_Inactive:
		{
			//It=std::find(m_vecSoundSpotsInactive.begin(), m_vecSoundSpotsInactive.end(), cs);
			It=m_lstSoundSpotsInactive.find(cs);
			if (It!=m_lstSoundSpotsInactive.end())
			{
				//if (((It-m_SoundSpotsInactive.begin())>=m_nLastInactiveListPos) && (m_nLastInactiveListPos>0))
				//	m_nLastInactiveListPos--;
				m_itLastInactivePos=m_lstSoundSpotsInactive.erase(It);
				m_nInactiveSoundSpotsSize--;				
			}
			break;
		}
	}
	cs->m_State=eSoundState_None;
	It=m_lstFadeUnderwaterSounds.find(cs);
	if (It!=m_lstFadeUnderwaterSounds.end())
		m_lstFadeUnderwaterSounds.erase(It);
}

//////////////////////////////////////////////////////////////////////
ISound* CSoundSystem::GetSound(int soundId)
{
	return (NULL);
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::PlaySound(int soundId)
{
	GUARD_HEAP;
	CSound *sound = (CSound *)GetSound(soundId);
	if (sound)	
		sound->Play();	
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::SetListener(const CCamera &cCam,const Vec3d &vVel)
{
	GUARD_HEAP;
	FUNCTION_PROFILER( GetSystem(),PROFILE_SOUND );

	m_cCam=cCam;

	Vec3d vPos=m_cCam.GetPos();
	if (GetLengthSquared(vPos)<1)
		return; // hasnt been set yet, but the update3d function has been called
	Vec3d vAngles=m_cCam.GetAngles();

	//position,left handed
	float pos[3];
	pos[0]=vPos.x;
	pos[1]=vPos.z;
	pos[2]=vPos.y;	 
	
	//listener orientation
	Vec3d FVec1=vAngles;

	//flip up/down
	Vec3d TVec1=FVec1;
	TVec1.x+=270;
	if (TVec1.x>360)
		TVec1.x-=360;
	
	//calculate forward and top vector
	FVec1=ConvertToRadAngles(FVec1);
	TVec1=ConvertToRadAngles(TVec1);
	
	//already normalized after conversion to radians
	FVec1.Normalize();
	TVec1.Normalize();
	
	if (m_pCVarDopplerEnable->GetIVal())
	{ 
		Vec3d vTempVel=(vPos-m_vPos);
		float fTimeDelta=m_pISystem->GetITimer()->GetFrameTime();

		vTempVel=vTempVel*(m_pCVarDopplerValue->GetFVal()/fTimeDelta);

		float fVel[3];
		fVel[0]=vTempVel.x;
		fVel[1]=vTempVel.z;
		fVel[2]=vTempVel.y;        

		//CS_3D_Listener_SetAttributes(pos, NULL, 0, 0, 1.0f, 0, 1.0f, 0);
		CS_3D_Listener_SetAttributes(pos, fVel,FVec1.x,FVec1.z,FVec1.y,TVec1.x,TVec1.z,TVec1.y);
	}
	else
	{
		CS_3D_Listener_SetAttributes(pos, NULL,FVec1.x,FVec1.z,FVec1.y,TVec1.x,TVec1.z,TVec1.y);
	};

	m_vPos=vPos;
	m_vForward=FVec1;
	m_vTop=TVec1;
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
void	CSoundSystem::Silence()
{	
	GUARD_HEAP;

	SetEaxListenerEnvironment(EAX_PRESET_OFF);

	// reset last listener vis area - just to be 100% sure we dont use an invalid vis area
	RecomputeSoundOcclusion(false,false,true);

	m_bValidPos=false;

	// reset sound-scaling
	for (int i=0;i<MAX_SOUNDSCALE_GROUPS;i++)
	{
		m_fSoundScale[i]=1.0f;
	}

	CS_StopSound(CS_FREE);
	//if (m_vecSounds.empty()) 
	//	return;
	SoundListItor It=m_vecSounds.begin();
	while(It!=m_vecSounds.end())
	{
		CSound *pSound=*It;
		pSound->FreeChannel();
		//if (pSound->m_nFlags & FLAG_SOUND_ACTIVELIST)
			//SetSoundActiveState(pSound,eSoundState_Inactive);
		++It;
	}
	//Update();
}

//////////////////////////////////////////////////////////////////////////
void	CSoundSystem::Pause(bool bPause,bool bResetVolume)
{
	m_bPause=bPause;
	SoundListItor It=m_vecSounds.begin();
	while (It!=m_vecSounds.end())
	{
		CSound *pSound=*It;
		if (pSound->m_nChannel>=0)		
		{			
			if (bPause)				 
				CS_SetPaused(pSound->m_nChannel,1);
			else
			{
				pSound->m_fChannelPlayTime=GetISystem()->GetITimer()->GetCurrTime();
				CS_SetPaused(pSound->m_nChannel,0);				
			}
		}
		++It;
	} //It
	
	if (bResetVolume)
	{
		// this is needed for instance when going back to menu
		// after setting the sound volume to 0. The sound volume
		// calculation takes the general volume into account,
		// so if it was set to 0, when going to menu the sliders
		// and buttons won't produce any sound, because the sound system
		// is not updated anymore so it doesn't have a chance to 
		// update the general volume. Hence we reset the volume to 1,
		// and the individual volume of sound and music sliders is set
		// by the menu scripts calling setvolume
		//m_fSFXResetVolume=m_pCVarSFXVolume->GetFVal();
		//m_pCVarSFXVolume->Set(1.0f);				
		m_bResetVolume=true;
		m_fSFXVolume=1.0f; //m_pCVarSFXVolume->GetFVal();				
	}
	
	if (!bPause && m_bResetVolume)
	{	
		// restore normal volume back
		//m_pCVarSFXVolume->Set(m_fSFXResetVolume);				
		m_bResetVolume=false;
		m_fSFXVolume=m_pCVarSFXVolume->GetFVal();
		//SetGroupScale(SOUNDSCALE_MISSIONHINT,m_fSFXVolume*0.65f);		
	}
}

//////////////////////////////////////////////////////////////////////////
void	CSoundSystem::Mute(bool bMute)
{
	GUARD_HEAP;
	if (bMute)
		m_nMuteRefCnt++;
	else
		m_nMuteRefCnt--;
	if (m_nMuteRefCnt<0)
		m_nMuteRefCnt=0;
	bool bSetMute=m_nMuteRefCnt!=0;
	CS_SetSFXMasterVolume(bSetMute ? 0 : 255);
//	CS_SetMute(CS_ALL, bSetMute);
}
 
//////////////////////////////////////////////////////////////////////
bool CSoundSystem::IsEAX(int version)
{
	return (true);
}

//////////////////////////////////////////////////////////////////////////
bool CSoundSystem::GetCurrentEaxEnvironment(int &nPreset, CS_REVERB_PROPERTIES &Props)
{
	nPreset=m_nLastEax;
	if (m_pLastEAXProps)
		memcpy(&Props,m_pLastEAXProps,sizeof(CS_REVERB_PROPERTIES));
	else
		memset(&Props,0,sizeof(CS_REVERB_PROPERTIES));
	return (true);
}

//////////////////////////////////////////////////////////////////////
bool CSoundSystem::SetEaxListenerEnvironment(int nPreset, CS_REVERB_PROPERTIES *tpProps, int nFlags)
{
	GUARD_HEAP;
	FUNCTION_PROFILER( GetSystem(),PROFILE_SOUND );

  if (!m_pCVarEnableSoundFX->GetIVal())
	{
		return (false);
	}
	
	if (nFlags==FLAG_SOUND_OUTDOOR)
	{
		m_EAXOutdoor.nPreset=nPreset;
		m_EAXOutdoor.EAX=*tpProps;
		m_pILog->Log("Setting global EAX outdoor");
		if (m_bInside)
			return (false); // set only when outside
	}

	//if (!nFlags)
	//	nFlags=FLAG_SOUND_INDOOR | FLAG_SOUND_OUTDOOR;

	/*
	bool bIndoor=(m_pVisArea!=NULL);	
	bool bIndoor=false; 
	I3DEngine	*p3dEngine=m_pISystem->GetI3DEngine();
	if (p3dEngine)
		bIndoor=p3dEngine->GetVisAreaFromPos(m_cCam.GetPos())!=NULL;
	*/

	bool bSet=false;
	/*
	if (nFlags & FLAG_SOUND_INDOOR)
	{
		if (tpProps)
		{
			m_EAXIndoor.nPreset=-1;
			m_EAXIndoor.EAX=*tpProps;
		}else
		{
			m_EAXIndoor.nPreset=nPreset;
		}
		if (m_bInside)
			bSet=true;
	}
	if (nFlags & FLAG_SOUND_OUTDOOR)
	{
		if (tpProps)
		{
			m_EAXOutdoor.nPreset=-1;
			m_EAXOutdoor.EAX=*tpProps;
		}else
		{
			m_EAXOutdoor.nPreset=nPreset;
		} 
		if (!m_bInside)
			bSet=true;
	}
	if (!bSet)
		return true;
	*/
	if (tpProps)
	{
		CS_Reverb_SetProperties(tpProps);
		m_nLastEax=-1;
		m_pLastEAXProps=tpProps;
	}
	else
	{
		if (m_nLastEax==nPreset)
			return (true); // already set
		switch (nPreset) 
		{
			case EAX_PRESET_GENERIC:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_GENERIC;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_PADDEDCELL:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_PADDEDCELL;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_ROOM:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_ROOM;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_BATHROOM:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_BATHROOM;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_LIVINGROOM:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_LIVINGROOM;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_STONEROOM:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_STONEROOM;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_AUDITORIUM:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_AUDITORIUM;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_CONCERTHALL:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_CONCERTHALL;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_CAVE:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_CAVE;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_ARENA:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_ARENA;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_HANGAR:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_HANGAR;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_CARPETTEDHALLWAY:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_CARPETTEDHALLWAY;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_HALLWAY:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_HALLWAY;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_STONECORRIDOR:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_STONECORRIDOR;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_ALLEY:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_ALLEY;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_FOREST:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_FOREST;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_CITY:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_CITY;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_MOUNTAINS:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_MOUNTAINS;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_QUARRY:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_QUARRY;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_PLAIN:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_PLAIN;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_PARKINGLOT:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_PARKINGLOT;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_SEWERPIPE:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_SEWERPIPE;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_UNDERWATER:
			{
				CS_REVERB_PROPERTIES pProps=CS_PRESET_UNDERWATER;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
			case EAX_PRESET_OFF:
			default:
			{
				CS_REVERB_PROPERTIES pProps=MY_CS_PRESET_OFF;
				CS_Reverb_SetProperties(&pProps);
				break;
			}
		}	
		m_nLastEax=nPreset;
		m_pLastEAXProps=NULL; // using one of pre-defined presets
	}
	return (true);
}

//////////////////////////////////////////////////////////////////////////
bool CSoundSystem::SetGroupScale(int nGroup, float fScale)
{
	GUARD_HEAP;
	if ((nGroup<0) || (nGroup>=MAX_SOUNDSCALE_GROUPS))
		return false;
	if (fabs(m_fSoundScale[nGroup]-fScale)<0.001f)
		return true;
	m_fSoundScale[nGroup]=fScale;
	//change volume of all looping/playing sounds
	//m_pILog->LogToConsole("scaling group for %s",cs->GetName());

	for (SoundListItor It=m_vecSounds.begin();It!=m_vecSounds.end();++It)
	{			 
		CSound *cs=(*It);
		if (cs->IsPlayingOnChannel())	
		{			
			int nVolume=cs->CalcSoundVolume(cs->m_nVolume,cs->m_fRatio);
			cs->ChangeVolume(nVolume);												
		}
	} //It

	return true;
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::SetMasterVolumeScale(float fScale, bool bForceRecalc)
{ 
	GUARD_HEAP;
	if ((!bForceRecalc) && ((fabs(fScale-m_fSoundScale[0]))<0.01f))
		return;
	SetGroupScale(0, fScale);
}

//////////////////////////////////////////////////////////////////////
void CSoundSystem::RemoveBuffer(SSoundBufferProps &sn)
{
	GUARD_HEAP;
	SoundBufferPropsMapItor itor=m_soundBuffers.find(sn);
	if(itor!=m_soundBuffers.end())
	{
		m_pILog->LogToFile("Removing sound [%s]",itor->first.sName.c_str());
		m_soundBuffers.erase(itor);
	}
	else
	{
		m_pILog->LogToFile("Warning Trying to remove sound [%s] NOT FOUND!!!",sn.sName.c_str());
	}
}

//////////////////////////////////////////////////////////////////////
void	CSoundSystem::GetSoundMemoryUsageInfo(size_t &nCurrentMemory,size_t &nMaxMemory)
{
	GUARD_HEAP;
#if (defined CS_VERSION_372) || (defined CS_VERSION_361)
  unsigned int tmpnCurrentMemory = nCurrentMemory;
  unsigned int tmpnMaxMemory = nMaxMemory;
	CS_GetMemoryStats(&tmpnCurrentMemory,&tmpnMaxMemory);
  nCurrentMemory = tmpnCurrentMemory;
  nMaxMemory = tmpnMaxMemory;
#else
  CS_GetMemoryStats(&nCurrentMemory,&nMaxMemory);
#endif
}

//////////////////////////////////////////////////////////////////////////
int	CSoundSystem::GetUsedVoices()
{
	GUARD_HEAP;
	return CS_GetChannelsPlaying();
}

///////////////////////////////////	///////////////////////////////////////
float	CSoundSystem::GetCPUUsage()
{
	GUARD_HEAP;
	return CS_GetCPUUsage();
}

//////////////////////////////////////////////////////////////////////////

float CSoundSystem::GetMusicVolume()
{
	return (m_fMusicVolume*m_fSoundScale[SOUNDSCALE_MISSIONHINT]*m_fSoundScale[SOUNDSCALE_DEAFNESS]);
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::CalcDirectionalAttenuation(Vec3d &Pos, Vec3d &Dir, float fConeInRadians)
{
	GUARD_HEAP;
	float fCosCone=(float)cry_cosf(fConeInRadians);
	if (IsEquivalent(Pos, m_DirAttPos, VEC_EPSILON) &&
			IsEquivalent(Dir, m_DirAttDir, VEC_EPSILON) &&
			(fCosCone==m_fDirAttCone))
	 return;
	m_DirAttPos=Pos;
	m_DirAttDir=Dir;
	m_fDirAttCone=fCosCone;
	m_fDirAttMaxScale=0.0f;
	//update position of all looping/playing sounds
	for (SoundListItor It=m_vecSounds.begin();It!=m_vecSounds.end();++It)
	{
		CSound *cs=(*It);
		if (cs->m_bPlaying || cs->IsPlayingVirtual())
			cs->UpdatePosition();
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::GetMemoryUsage(class ICrySizer* pSizer)
{
	GUARD_HEAP;
	size_t nSize = sizeof(*this);
	//if (!pSizer->AddObject(this, sizeof(*this)+nCurrentAlloced))
	//	return;
	for (SoundListItor SoundIt=m_vecSounds.begin();SoundIt!=m_vecSounds.end();++SoundIt)
	{
		CSound *pSound=(*SoundIt);
		pSound->GetMemoryUsage(pSizer);
	}

	for (SoundBufferPropsMap::iterator it = m_soundBuffers.begin(); it != m_soundBuffers.end(); ++it)
		nSize += sizeof (*it) + sizeof(it->first) + it->first.sName.size() + 1;

	pSizer->AddObject(this, nSize);

	{
		SIZER_COMPONENT_NAME(pSizer, "FMOD");
		size_t nCurrentAlloced;
		size_t nMaxAlloced;
#if (defined CS_VERSION_372) || (defined CS_VERSION_361)
    unsigned int tmpnCurrentMemory;
    unsigned int tmpnMaxMemory;
	  CS_GetMemoryStats(&tmpnCurrentMemory,&tmpnMaxMemory);
    nCurrentAlloced = tmpnCurrentMemory;
    nMaxAlloced = tmpnMaxMemory;
#else
 		CS_GetMemoryStats(&nCurrentAlloced, &nMaxAlloced);
#endif

		//CS_GetMemoryStats(&nCurrentAlloced, &nMaxAlloced);
		if (!pSizer->AddObject(&CS_Init, nCurrentAlloced))
			return;
	}
}

//////////////////////////////////////////////////////////////////////////
int CSoundSystem::SetMinSoundPriority( int nPriority )
{
	int nPrev = m_nMinSoundPriority;
	m_nMinSoundPriority = nPriority;
	return nPrev;
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::LockResources()
{
	for (SoundBufferPropsMapItor It=m_soundBuffers.begin();It!=m_soundBuffers.end();It++)
	{			
		CSoundBuffer *pBuf = It->second;
		m_lockedResources.push_back( pBuf );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundSystem::UnlockResources()
{
	m_lockedResources.clear();
}

//////////////////////////////////////////////////////////////////////////
bool CSoundSystem::IsEnabled()
{
	return m_pCVARSoundEnable->GetIVal() != 0;
}

#endif

extern "C"
{
	void CheckMem(void* pData, size_t nSize)
	{
		assert (0 == IsBadReadPtr(pData, nSize));
	}
}