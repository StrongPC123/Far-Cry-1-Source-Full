#include "StdAfx.h"
#include <algorithm>
#include "SoundBuffer.h"
#include "SoundSystem.h"
#include "Sound.h"
#include <IGame.h>

CSoundBuffer::CSoundBuffer(CSoundSystem *pSoundSystem, SSoundBufferProps &Props) : m_Props(Props)
{
	m_pSoundSystem=pSoundSystem; 
	m_Data.m_pData=NULL;
	m_Type=btNONE;
	m_nRef=0;		
	m_nBaseFreq=44100;
	m_fLength=0.0f;
	m_pReadStream=NULL;
	m_bLoadFailure=false;
	m_bLooping=false;
	m_bCallbackIteration=false;
	m_vecLoadReq.clear();
	m_vecLoadReqToRemove.clear();
	m_vecLoadReqToAdd.clear();
}

CSoundBuffer::~CSoundBuffer()
{
	if (m_pReadStream!=NULL)
	{
		m_pReadStream->Abort();
		m_pReadStream=NULL;
	}
	DestroyData();
}

int CSoundBuffer::AddRef()
{
	return ++m_nRef;
}

int CSoundBuffer::Release()
{
	--m_nRef;
	if(m_nRef<=0)
	{
		m_pSoundSystem->RemoveBuffer(m_Props);
		delete this;
	}
	return 0;
}

#if defined(_DEBUG) && defined(WIN64)
static __int64 g_numSBSetSample = 0;
#endif


void CSoundBuffer::SetSample(CS_SAMPLE *pPtr)
{
#if defined(_DEBUG) && defined(WIN64)
	++g_numSBSetSample;
#endif

	if (pPtr && (!m_Data.m_pSample))
	{
		m_pSoundSystem->BufferLoaded(this);
	}
	else
	if ((!pPtr) && m_Data.m_pSample)
	{
		m_pSoundSystem->BufferUnloaded(this);
	}
	m_Data.m_pSample=pPtr;
	m_Type=(pPtr!=NULL) ? btSAMPLE : btNONE;
}

void CSoundBuffer::SetStream(CS_STREAM *pPtr)
{
	GUARD_HEAP;

	if (pPtr && (!m_Data.m_pStream))
		m_pSoundSystem->BufferLoaded(this);
	else
	if ((!pPtr) && m_Data.m_pStream)
		m_pSoundSystem->BufferUnloaded(this);
	m_Data.m_pStream=pPtr;
	m_Type=(pPtr!=NULL) ? btSTREAM : btNONE;
}

void CSoundBuffer::UpdateCallbacks()
{
	if (m_bCallbackIteration)
		return;
	GUARD_HEAP;
	for (TBufferLoadReqVecIt It=m_vecLoadReqToRemove.begin();It!=m_vecLoadReqToRemove.end();++It)
	{
		TBufferLoadReqVecIt RemIt=std::find(m_vecLoadReq.begin(), m_vecLoadReq.end(), *It);
		if (RemIt!=m_vecLoadReq.end())
			m_vecLoadReq.erase(RemIt);
	}
	m_vecLoadReqToRemove.clear();
	for (TBufferLoadReqVecIt It=m_vecLoadReqToAdd.begin();It!=m_vecLoadReqToAdd.end();++It)
	{
		m_vecLoadReq.push_back(*It);
	}
	m_vecLoadReqToAdd.clear();
}

#define CALLBACK_FUNC(_func) \
	m_bCallbackIteration=true; \
	for (TBufferLoadReqVecIt It=m_vecLoadReq.begin();It!=m_vecLoadReq.end();++It) \
	{ \
		(*It)->_func(); \
	} \
	m_vecLoadReq.clear(); \
	m_bCallbackIteration=false; \
	UpdateCallbacks();

void CSoundBuffer::SoundLoaded()
{
	//TRACE("Sound-Streaming for %s SUCCEEDED.", m_Props.sName.c_str());
	//FUNCTION_PROFILER( m_pSoundSystem->GetSystem(),PROFILE_SOUND );
	switch (m_Type)
	{
		case btSAMPLE:
		{
			GUARD_HEAP;
			//get the base frequence
			CS_Sample_GetDefaults(m_Data.m_pSample, &m_nBaseFreq, NULL, NULL, NULL);
			// set default values: ignore volume and pan, but sets frequency and priority
			CS_Sample_SetDefaults(m_Data.m_pSample, m_nBaseFreq, -1, -1, 0);
			m_fLength=(float)CS_Sample_GetLength(m_Data.m_pSample)/(float)m_nBaseFreq;
			break;
		}
		case btSTREAM:
		{
			GUARD_HEAP;
			m_fLength=(float)CS_Stream_GetLengthMs(m_Data.m_pStream)/1000.0f;
			break;
		}
	}
	m_pReadStream=NULL;
	if (m_pSoundSystem->m_pCVarDebugSound->GetIVal()==3)
		m_pSoundSystem->m_pILog->Log("file %s, loaded \n", m_Props.sName.c_str());
	// broadcast OnBufferLoaded
	GUARD_HEAP;
	CALLBACK_FUNC(OnBufferLoaded);
/*	m_bCallbackIteration=true;
	for (TBufferLoadReqVecIt It=m_vecLoadReq.begin();It!=m_vecLoadReq.end();++It)
	{
		CSound *pSound=(*It);
		pSound->OnBufferLoaded();
	}
	m_vecLoadReq.clear();
	m_bCallbackIteration=false;
	UpdateCallbacks();*/
}

void CSoundBuffer::LoadFailed()
{
	m_pSoundSystem->GetSystem()->Warning( VALIDATOR_MODULE_SOUNDSYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,"Sound %s failed to load", m_Props.sName.c_str() );
	m_pReadStream=NULL;

	// broadcast OnBufferLoadFailed
	CALLBACK_FUNC(OnBufferLoadFailed);
/*	m_bCallbackIteration=true;
	int n;
	for (TBufferLoadReqVecIt It=m_vecLoadReq.begin();It!=m_vecLoadReq.end();++It)
	{
		CSound *pSound=(*It);
		pSound->OnBufferLoadFailed();
	}
	m_vecLoadReq.clear();
	m_bCallbackIteration=false;
	UpdateCallbacks();*/
}

void CSoundBuffer::RemoveFromLoadReqList(CSound *pSound)
{
	GUARD_HEAP;
	m_vecLoadReqToRemove.push_back(pSound);
	UpdateCallbacks();
}

bool CSoundBuffer::Load(bool bLooping, CSound *pSound)
{
	GUARD_HEAP;
	FUNCTION_PROFILER( m_pSoundSystem->GetSystem(),PROFILE_SOUND );
	if (m_Data.m_pData)
	{
		if (pSound)
			pSound->OnBufferLoaded();
		return true; // already loaded
	}
	if (LoadFailure())
	{
		if (pSound)
			pSound->OnBufferLoadFailed();
		return false; //already tried to open the file before
	}
	m_vecLoadReqToAdd.push_back(pSound);
	UpdateCallbacks();
	if (!NotLoaded())
		return true;
	if (m_Props.sName.empty())
		return false;
	m_bLooping=bLooping;
	//priority for streaming files
	if (m_Props.nFlags & FLAG_SOUND_STREAM)
	{		 
		// add the game path since this file is not opened via the pak
		// file system
		GUARD_HEAP;
		CS_STREAM *pSoundStream=NULL;		
		
		// check for a MOD first
		const char *szPrefix=NULL;
#ifndef _ISNOTFARCRY
		IGameMods *pMods=m_pSoundSystem->GetSystem()->GetIGame()->GetModsInterface();
		if (pMods)		
			szPrefix=pMods->GetModPath(m_Props.sName.c_str());
#endif
				
		if (szPrefix)
		{
#ifndef CS_VERSION_372
			pSoundStream=CS_Stream_OpenFile(szPrefix, GetFModFlags(m_bLooping), 0);
#else
			// for 3.7
			pSoundStream=CS_Stream_Open(szPrefix, GetFModFlags(m_bLooping), 0,0);
#endif
			if (!pSoundStream) 
			{
				// if not found in the MOD folder, fallback to the default one and 
				// try to open it from there				

				// for 3.7 support
				const char * pFileName = m_Props.sName.c_str();
#ifndef CS_VERSION_372
        pSoundStream=CS_Stream_OpenFile(pFileName, GetFModFlags(m_bLooping), 0);
#else
				pSoundStream=CS_Stream_Open(pFileName, GetFModFlags(m_bLooping), 0,0);
#endif
      }			
		}
		else
    {
			// no MOD folder
			// 3.7 changes
      const char * pFileName = m_Props.sName.c_str();
      int fmf = GetFModFlags(m_bLooping);
#ifndef CS_VERSION_372
			pSoundStream=CS_Stream_OpenFile(pFileName, fmf, 0);
#else
			pSoundStream=CS_Stream_Open(m_Props.sName.c_str(), GetFModFlags(m_bLooping), 0,0);
#endif
    }

		if (!pSoundStream)
		{
			//cannot be loaded
			m_pSoundSystem->m_pILog->LogToFile("Warning: Cannot load streaming sound %s\n", m_Props.sName.c_str());
			m_bLoadFailure=true;
			return false;
		}
		//set the sound buffer
		SetStream(pSoundStream);
		SoundLoaded();
	}
	else
	{
		ASSERT(m_pSoundSystem->m_pStreamEngine);
		//TRACE("Starting Sound-Streaming for %s.", m_Props.sName.c_str());
		m_pReadStream=m_pSoundSystem->m_pStreamEngine->StartRead("SoundSystem", m_Props.sName.c_str(), this);
		if (pSound->m_nFlags & FLAG_SOUND_LOAD_SYNCHRONOUSLY)
		{
			if (m_pReadStream)
				m_pReadStream->Wait();
		}
		else
		{
			if (m_pReadStream->IsFinished())
				m_pReadStream=NULL;
		}
		
		// Placeholder sound.
		if (m_pSoundSystem->m_pCVarDebugSound->GetIVal() == 2)
		{
			if (m_pReadStream)
				m_pReadStream->Wait();
			if (m_bLoadFailure)
			{
				m_pReadStream = NULL;
				m_bLoadFailure = false;
				m_Props.sName = "Sounds/error.wav";
				return Load( m_bLooping,pSound );
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CSoundBuffer::WaitForLoad()
{
	GUARD_HEAP;
	if (!Loading())
		return true;
	m_pReadStream->Wait();
	return !LoadFailure();
}

//////////////////////////////////////////////////////////////////////////
void CSoundBuffer::AbortLoading()
{
	if (m_pReadStream!=NULL)
	{
		m_pReadStream->Abort();
		m_pReadStream=NULL;
		LoadFailed();
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundBuffer::DestroyData()
{
	GUARD_HEAP;
	if (m_Data.m_pData)
	{
		FUNCTION_PROFILER( m_pSoundSystem->GetSystem(),PROFILE_SOUND );
		switch (m_Type)
		{
			case btSAMPLE:	CS_Sample_Free(m_Data.m_pSample);		SetSample(NULL);	break;
			case btSTREAM:	CS_Stream_Close(m_Data.m_pStream);	SetStream(NULL);	break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CSoundBuffer::GetFModFlags(bool bLooping)
{
	int nFModFlags=0;
	if (m_Props.nFlags & FLAG_SOUND_3D)
		nFModFlags|=CS_HW3D;
	if (m_Props.nFlags & FLAG_SOUND_STEREO)
		nFModFlags|=CS_STEREO;
	if (m_Props.nFlags & FLAG_SOUND_16BITS)
		nFModFlags|=CS_16BITS;
	if (m_Props.nFlags & FLAG_SOUND_2D)
		nFModFlags|=CS_2D;
	//if not flag specified or if a streaming sound, set default flags
	if ((!nFModFlags) || (m_Props.nFlags & FLAG_SOUND_STREAM))
		nFModFlags|=CS_2D | CS_STEREO;
	nFModFlags|=(bLooping) ? CS_LOOP_NORMAL : CS_LOOP_OFF;
	return nFModFlags;
}

//////////////////////////////////////////////////////////////////////////
void CSoundBuffer::StreamOnComplete(IReadStream *pStream, unsigned nError)
{
	GUARD_HEAP;
	FUNCTION_PROFILER( m_pSoundSystem->GetSystem(),PROFILE_SOUND );
	if (nError)
	{
		m_bLoadFailure=true;
		LoadFailed();
		return;
	}
	//use unmanaged channels/indexes, any other type creates problems
#ifndef CS_VERSION_372
	CS_SAMPLE *pSample=CS_Sample_Load(CS_UNMANAGED, (const char*)pStream->GetBuffer(), GetFModFlags(m_bLooping) | CS_LOADMEMORY, pStream->GetBytesRead());
#else
	// 3.7
	CS_SAMPLE *pSample=CS_Sample_Load(CS_UNMANAGED, (const char*)pStream->GetBuffer(), GetFModFlags(m_bLooping) | CS_LOADMEMORY, 0, pStream->GetBytesRead());
#endif
  if (!pSample)
	{
		m_pSoundSystem->m_pILog->LogToFile("Warning: Cannot load sample sound %s\n", m_Props.sName.c_str());
		m_bLoadFailure=true;
		LoadFailed();
		return;
	}		
	//set the sound source
	SetSample(pSample);
  //CS_Sample_SetMode(pSample, m_bLooping ? CS_LOOP_NORMAL : CS_LOOP_OFF);
  CS_Sample_SetMode(pSample, GetFModFlags(m_bLooping));
	SoundLoaded();
	//TRACE("Sound-Streaming for %s finished.", m_Props.sName.c_str());
}