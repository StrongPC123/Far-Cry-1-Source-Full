#include "StdAfx.h"
#include "musicsystem.h"
#include <CrySizer.h>
#include <ISystem.h>
#include "PatternDecoder.h"
#include "MusicLoadSink.h"
/*
 *	INFO: You should read the document CryMusicSystem.doc first to know about the terminology used.
 */

/* streaming-callback called by fmod.
*/
//signed char __cdecl _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, INT_PTR nParam)
#ifndef CS_VERSION_372
#ifdef CS_VERSION_361
signed char F_CALLBACKAPI _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, int nParam)
#else
signed char F_CALLBACKAPI _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, INT_PTR nParam)
#endif
#else
signed char F_CALLBACKAPI _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, void * nParam)
#endif
{
	if (!nParam)
		return FALSE;
	CMusicSystem *pMusicSystem=(CMusicSystem*)nParam;
	return pMusicSystem->StreamingCallback(pStream, pBuffer, nLength);
}

/* constructor
*/
CMusicSystem::CMusicSystem(ISystem *pSystem)
{
	InitializeCriticalSection(&m_CS);
	m_pSystem=pSystem;
	m_pTimer=m_pSystem->GetITimer();
	m_pLog=m_pSystem->GetILog();
	m_pSoundSystem=m_pSystem->GetISoundSystem();
	if (m_pSoundSystem)
		m_fMasterVolume=m_pSoundSystem->GetMusicVolume();
	else
		m_fMasterVolume=0.0f;
	m_nSampleRate=44100;
//	m_fLatency=0.25f;
	m_fLatency=0.1f;//to let it peak less
	m_pStream=NULL;
	m_nChannel=-1;
	m_bPlaying = false;
	m_mapPatterns.clear();
	m_mapThemes.clear();
	m_pCurrPattern=NULL;
	m_pNextPattern=NULL;
	m_nLayeredRhythmicPatterns=0;
	m_nLayeredIncidentalPatterns=0;
	m_pMixBuffer=NULL;
	m_pThemeOverride=NULL;
	m_pCurrTheme=NULL;
	m_pNextTheme=NULL;
	m_pCurrMood=NULL;
	m_pNextMood=NULL;
	m_bForcePatternChange=false;
	m_fDefaultMoodTime=0.0f;
	m_sDefaultMood="";
	m_pSink=NULL;
	m_pDataPtr=NULL;
	m_bDataLoaded=false;
	m_bPause=false;
	m_bBridging=false;
	m_bCurrPatternIsBridge=false;
	m_fCurrCrossfadeTime=DEFAULT_CROSSFADE_TIME;
	m_bOwnMusicData = false;

	m_pCVarDebugMusic=pSystem->GetIConsole()->CreateVariable("s_DebugMusic","0",0,
		"Toggles music-debugging on and off.\n"
		"Usage: s_DebugMusic [0/1]\n"
		"Default is 0 (off). Set to 1 to debug music.");	
	m_pCVarMusicEnable=pSystem->GetIConsole()->CreateVariable("s_MusicEnable","1",VF_DUMPTODISK,"enable/disable music");
	m_pCVarMusicMaxSimultaniousPatterns = pSystem->GetIConsole()->CreateVariable("s_MusicMaxPatterns","12",VF_DUMPTODISK,
		"Max simultaniously playing music patterns.\n"
	);
	m_pCVarMusicStreamedData = 
		pSystem->GetIConsole()->CreateVariable( "s_MusicStreamedData", "0", VF_DUMPTODISK,
		"Data used for streaming music data.\n0 - (AD)PCM\n1 - OGG" );

	m_musicEnable = 1;
	Init();
}

/* destructor; make sure everything gets unloaded
*/
CMusicSystem::~CMusicSystem()
{
	Unload();
	DeleteCriticalSection(&m_CS);
}

/* destroy system
*/
void CMusicSystem::Release()
{
	Shutdown();
	delete this;
}

/* set up notification-sink
*/
IMusicSystemSink* CMusicSystem::SetSink(IMusicSystemSink *pSink)
{
	IMusicSystemSink *pOldSink=m_pSink;
	m_pSink=pSink;
	return pOldSink;
}

/* initialize system and create fmod-streaming callback
*/
bool CMusicSystem::Init()
{
	GUARD_HEAP;
	CSmartCriticalSection SmartCriticalSection(m_CS);
	EnableEventProcessing(true);

	m_RandGen.Seed(GetTickCount() & 0xfffffffe);
	m_nBytesPerSample=4;	// always use STEREO 16 BIT

	m_pMixBuffer=new char[(int)((float)m_nSampleRate*m_fLatency)*m_nBytesPerSample];
#if (defined CS_VERSION_372)
  m_pStream=CS_Stream_Create(_StreamingCallback, (int)((float)m_nSampleRate*m_fLatency)*m_nBytesPerSample, CS_STEREO | CS_16BITS | CS_SIGNED | CS_2D, 44100, (void *)this);
#elif  defined CS_VERSION_361
	m_pStream=CS_Stream_Create(_StreamingCallback, (int)((float)m_nSampleRate*m_fLatency)*m_nBytesPerSample, CS_STEREO | CS_16BITS | CS_SIGNED | CS_2D, 44100, (int)this);
#else
  m_pStream=CS_Stream_Create(_StreamingCallback, (int)((float)m_nSampleRate*m_fLatency)*m_nBytesPerSample, CS_STEREO | CS_16BITS | CS_SIGNED | CS_2D, 44100, (INT_PTR)this);
#endif
  if (!m_pStream)
		return false;;

	return true;
}

/* counterpart of init(); destroy stream and cleanup allocated mem.
*/
void CMusicSystem::Shutdown()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	Silence();
	StopPlaying();
	if (m_pStream)
	{
		GUARD_HEAP;
		CS_Stream_Close(m_pStream);
	}
	m_pStream=NULL;
	m_nChannel=-1;
	m_bPlaying = false;
	if (m_pMixBuffer)
		delete[] m_pMixBuffer;
	m_pMixBuffer=NULL;
	FlushPatterns();
}

/* push log-message in internal list, so it can be outputted on next update-cycle;
	this has been done due to multi-threading issues with the logging functions.
*/
void CMusicSystem::LogMsg(const char *pszFormat, ...)
{
	if (m_pLog && m_pCVarDebugMusic->GetIVal())
	{
		va_list ArgList;
		char szBuffer[2048];
		va_start(ArgList, pszFormat);
		vsprintf(szBuffer, pszFormat, ArgList);
		va_end(ArgList);
		m_vecLog.push_back(szBuffer);
	}
}

/* flush all log-messages and delete internal list
*/
void CMusicSystem::FlushLog()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (m_pLog && m_pCVarDebugMusic->GetIVal())
	{
		for (TStringVecIt It=m_vecLog.begin();It!=m_vecLog.end();++It)
		{
			m_pLog->Log((*It).c_str());
		}
	}
	m_vecLog.clear();
}

/* add (create) pattern to the music-system and create/open its decoder instance
*/
CMusicPattern* CMusicSystem::AddPattern(const char *pszName, const char *pszFilename)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	CMusicPattern *pPattern=new CMusicPattern(this, pszName,pszFilename);
	// convert name to lowercase
	//string sName=pszName;
	//strlwr((char*)sName.c_str());
	// Check if file exist.
	/*
	FILE *pFile = m_pSystem->GetIPak()->FOpen(pszFilename, "rb");
	if (!m_FileAccess.pFile)
		return false;
		*/

	/*
	if (!pPattern->Open(pszFilename))
	{
		delete pPattern;
		LogMsg("[MusicSystem] WARNING: Cannot load music-pattern %s (%s) !", pszName, pszFilename);
		return NULL;
	}
	*/
	m_mapPatterns.insert(TPatternMapIt::value_type(pszName, pPattern));
	return pPattern;
}

/* release all patterns
*/
bool CMusicSystem::FlushPatterns()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (m_bOwnMusicData)
		ReleaseData();
	m_pDataPtr=NULL;
	m_mapThemes.clear();
	m_pThemeOverride=NULL;
	m_pCurrTheme=NULL;
	m_pNextTheme=NULL;
	m_pCurrMood=NULL;
	m_pNextMood=NULL;
	m_vecPlayingPatterns.clear();
	m_mapPatterns.clear();
	m_pCurrPattern=NULL;
	m_pNextPattern=NULL;
	m_nLayeredRhythmicPatterns=0;
	m_nLayeredIncidentalPatterns=0;
	m_bForcePatternChange=false;
	m_fDefaultMoodTime=0.0f;
	m_sDefaultMood="";
	m_bDataLoaded=false;
	m_bPause=false;
	m_bBridging=false;
	m_bCurrPatternIsBridge=false;

	for (TPatternMapIt It=m_mapPatterns.begin();It!=m_mapPatterns.end();++It)
	{
		CMusicPattern *pPattern=It->second;
		delete pPattern;
	}
	return true;
}

/* start streaming (so callback will be called)
*/
bool CMusicSystem::StartPlaying()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!m_bDataLoaded)
		return false;
	if (m_bPause)
		return true;
	if (m_nChannel>=0)
		return false;
	if (m_pSoundSystem)
		m_fMasterVolume=m_pSoundSystem->GetMusicVolume();
	if (m_fMasterVolume<=0)
		return true;
	GUARD_HEAP;

	m_bPlaying = true;

	m_nChannel=CS_Stream_Play(CS_FREE, m_pStream);
	if (m_nChannel<0)
	{
		FlushPatterns();
		return false;
	}
	CS_SetVolume(m_nChannel, (int)(m_fMasterVolume*255.0f));
	return true;
}

/* stop streaming; streaming-callback will not be called anymore
*/
bool CMusicSystem::StopPlaying()
{
	if (m_nChannel<0)
		return false;
	CSmartCriticalSection SmartCriticalSection(m_CS);
	m_bPlaying = false;
	GUARD_HEAP;
	CS_Stream_Stop(m_pStream);
	m_nChannel=-1;
	return true;
}

/* set music-system data (eg. read from lua-file by gamecode)
	we convert all names to lowercase to make them not case-sensitive (faster lookup)
	we start streaming
*/
bool CMusicSystem::SetData(SMusicData *pMusicData,bool bNoRelease)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	m_bOwnMusicData = !bNoRelease;
	StopPlaying();
 	FlushPatterns();
	m_pDataPtr=pMusicData;

	if (!m_pDataPtr)
		return true;

	// create patterns
	for (TPatternDefVecIt It=m_pDataPtr->vecPatternDef.begin();It!=m_pDataPtr->vecPatternDef.end();++It)
	{
		SPatternDef *pPatternDef=(*It);
		CMusicPattern *pPattern=AddPattern(pPatternDef->sName.c_str(), pPatternDef->sFilename.c_str());
		if (!pPattern)
		{
			m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,pPatternDef->sFilename.c_str(),
													"Music file %s failed to load",pPatternDef->sFilename.c_str() );
			continue;
			//FlushPatterns(bNoRelease);
			//return false;
		}
		for (TIntVecIt IntIt=pPatternDef->vecFadePoints.begin();IntIt!=pPatternDef->vecFadePoints.end();++IntIt)
		{
			pPattern->AddFadePoint(*IntIt);
		}
		pPattern->SetLayeringVolume(pPatternDef->nLayeringVolume);
	}

	// copy themes
	{
		m_mapThemes.clear();
		for (TThemeMap::iterator it = m_pDataPtr->mapThemes.begin(); it != m_pDataPtr->mapThemes.end(); ++it)
		{
			m_mapThemes.insert( TThemeMap::value_type(it->first,it->second) );
		}
	}

	/*
	// make all theme/mood/pattern names lowercase
	for (TThemeMapIt ThemeIt=m_mapThemes.begin();ThemeIt!=m_mapThemes.end();++ThemeIt)
	{
		SMusicTheme *pTheme=ThemeIt->second;
		strlwr((char*)ThemeIt->first.c_str());	// convert theme key
		strlwr((char*)pTheme->sName.c_str());	// convert theme name
		strlwr((char*)pTheme->sDefaultMood.c_str());	// convert default mood
		for (TMoodMapIt MoodIt=pTheme->mapMoods.begin();MoodIt!=pTheme->mapMoods.end();++MoodIt)
		{
			SMusicMood *pMood=MoodIt->second;
			pMood->pCurrPatternSet=NULL;
			strlwr((char*)MoodIt->first.c_str());	// convert mood key
			strlwr((char*)pMood->sName.c_str());	// convert mood name
			for (TPatternSetVecIt PatternSetIt=pMood->vecPatternSets.begin();PatternSetIt!=pMood->vecPatternSets.end();++PatternSetIt)
			{
				SMusicPatternSet *pPatterSet=(*PatternSetIt);
				for (TPatternDefVecIt PatternIt=pPatterSet->vecMainPatterns.begin();PatternIt!=pPatterSet->vecMainPatterns.end();++PatternIt)
				{
					SMusicPatternInfo &PatternInfo=(*PatternIt);
					strlwr((char*)PatternInfo.sName.c_str());	// convert pattern name
				}
				for (TPatternDefVecIt PatternIt=pPatterSet->vecRhythmicPatterns.begin();PatternIt!=pPatterSet->vecRhythmicPatterns.end();++PatternIt)
				{
					SMusicPatternInfo &PatternInfo=(*PatternIt);
					strlwr((char*)PatternInfo.sName.c_str());	// convert pattern name
				}
				for (TPatternDefVecIt PatternIt=pPatterSet->vecIncidentalPatterns.begin();PatternIt!=pPatterSet->vecIncidentalPatterns.end();++PatternIt)
				{
					SMusicPatternInfo &PatternInfo=(*PatternIt);
					strlwr((char*)PatternInfo.sName.c_str());	// convert pattern name
				}
			}
		}
		for (TThemeBridgeMapIt BridgeIt=pTheme->mapBridges.begin();BridgeIt!=pTheme->mapBridges.end();++BridgeIt)
		{
			strlwr((char*)BridgeIt->first.c_str());	// convert mood key
			strlwr((char*)BridgeIt->second.c_str());	// convert mood value
		}
	}
	*/
	m_bDataLoaded=true;
	// start playing the stream...
	if (m_nChannel<0)
		StartPlaying();
	return true;
}



bool CMusicSystem::LoadMusicDataFromLUA(IScriptSystem* pScriptSystem, const char *pszFilename)
{
	SMusicData *pMusicData=new SMusicData();
	pScriptSystem->SetGlobalToNull("DynamicMusic");
	if (!pScriptSystem->ExecuteFile(pszFilename, true, true))
		return false;
	_SmartScriptObject pObj(pScriptSystem, true);
	if (!pScriptSystem->GetGlobalValue("DynamicMusic", pObj))
		return false;
	CMusicLoadSink LoadSink(pMusicData, pScriptSystem, &pObj);
	pObj->Dump(&LoadSink);
	pScriptSystem->SetGlobalToNull("DynamicMusic");
	if (!SetData(pMusicData))
		return false;
	return true;
}


void CMusicSystem::ReleaseData()
{
	if (!m_bOwnMusicData)
		return;
	SMusicData *pData = m_pDataPtr;
	if (!pData)
		return;

	// release pattern-defs
	for (TPatternDefVecIt PatternIt=pData->vecPatternDef.begin();PatternIt!=pData->vecPatternDef.end();++PatternIt)
	{
		SPatternDef *pPattern=(*PatternIt);
		delete pPattern;
	}
	// release themes/moods
	for (TThemeMapIt ThemeIt=pData->mapThemes.begin();ThemeIt!=pData->mapThemes.end();++ThemeIt)
	{
		SMusicTheme *pTheme=ThemeIt->second;
		for (TMoodMapIt MoodIt=pTheme->mapMoods.begin();MoodIt!=pTheme->mapMoods.end();++MoodIt)
		{
			SMusicMood *pMood=MoodIt->second;
			for (TPatternSetVecIt PatternSetIt=pMood->vecPatternSets.begin();PatternSetIt!=pMood->vecPatternSets.end();++PatternSetIt)
			{
				SMusicPatternSet *pPatternSet=(*PatternSetIt);
				delete pPatternSet;
			}
			delete pMood;
		}
		delete pTheme;
	}
	delete pData;
}




/* stop streaming and unload all data from the system
*/
void CMusicSystem::Unload()
{
	// stop the stream...
	CSmartCriticalSection SmartCriticalSection(m_CS);
	Silence();
	StopPlaying();
	FlushPatterns();
}

/* pause/resume streaming
*/
void CMusicSystem::Pause(bool bPause)
{
	if (m_bPause!=bPause)
	{
		m_bPause=bPause;
		if (m_bPause)
			StopPlaying();
		else
			StartPlaying();
	}
}

/* get pointer to current mood descriptor
*/
SMusicMood* CMusicSystem::GetMood(SMusicTheme *pTheme, const char *pszMood)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!pTheme)
		return NULL;
	TMoodMapIt It=pTheme->mapMoods.find(pszMood);
	if (It==pTheme->mapMoods.end())
		return NULL;
	return It->second;
}

/* reset the theme override
*/
bool CMusicSystem::ResetThemeOverride()
{
	m_pThemeOverride=NULL;
	return true;
}

/* set the current theme by pointer to descriptor
*/
bool CMusicSystem::SetTheme(SMusicTheme *pNewTheme, bool bOverride)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (pNewTheme==m_pCurrTheme)
		return true;	// do nothing since we're already in the requested theme
	if (!((m_pCurrTheme==m_pNextTheme) || (pNewTheme)))	// only switch to the new theme if there was no switch in this frame yet OR the new theme is NOT null
		return true;
	if (pNewTheme)
		LogMsg("[MusicSystem] Setting theme to %s", pNewTheme->sName.c_str());
	else
		LogMsg("[MusicSystem] Setting theme to NULL");
	if (bOverride)
	{
		m_pThemeOverride=pNewTheme;
		m_pNextTheme=m_pCurrTheme;
	}else
	{
		m_pNextTheme=pNewTheme;
	}
	return true;
}

/* set the current theme by name
*/
bool CMusicSystem::SetTheme(const char *pszTheme, bool bOverride)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	string sTheme=pszTheme;
	if (strlen(pszTheme)==0)		// no theme
		return SetTheme((SMusicTheme*)NULL, false);
	TThemeMapIt It=m_mapThemes.find(sTheme);
	if (It==m_mapThemes.end())
		return false;	// theme not found
	SMusicTheme *pNewTheme=It->second;
	return SetTheme(pNewTheme, bOverride);
}

/* set the current mood by pointer to descriptor
*/
bool CMusicSystem::SetMood(SMusicMood *pNewMood)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!pNewMood)
	{
		m_pCurrMood=pNewMood;
		m_pNextMood=NULL;
		m_pNextPattern=NULL;
		LogMsg("[MusicSystem] Setting mood to <none>");
		return true;
	}
	if (pNewMood==m_pCurrMood)
		return true;	// do nothing since we're already in the requested mood
	LogMsg("[MusicSystem] Setting mood to %s", pNewMood->sName.c_str());
	// adjust fading time accourding to new pattern
	m_fCurrCrossfadeTime=(double)pNewMood->fFadeOutTime;

	if (!UpdateCurrentPatternSet(pNewMood, 0, true))
	{
		LogMsg("[MusicSystem] WARNING: Unable to find pattern-set in mood %s!", pNewMood->sName.c_str() );
		return false;
	}
	m_pNextPattern=ChoosePattern(pNewMood);
	if (!m_pNextPattern)
	{
		LogMsg("[MusicSystem] WARNING: Unable to find pattern in mood %s ", pNewMood->sName.c_str() );
		return false;
	}
	m_bForcePatternChange=(m_pCurrMood!=NULL) && (m_pCurrMood!=pNewMood);	// try to get into the new mood asap if we're changing the mood
	m_pCurrMood = pNewMood;
	return true;
}

/* set the current mood by name
*/
bool CMusicSystem::SetMood(const char *pszMood)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	string sMood=pszMood;
	
	SMusicTheme *pTheme = m_pCurrTheme;
	if (m_pNextTheme)
		pTheme = m_pNextTheme;
	if (!pTheme)
		return false;	// no theme set

	if (strlen(pszMood)==0)
	{
		SetMood((SMusicMood*)NULL);
		return true;
	}
	TMoodMapIt It = pTheme->mapMoods.find(sMood);
	if (It==pTheme->mapMoods.end())
		return false;	// mood not found
	SMusicMood *pNewMood=It->second;
	if (m_pNextTheme)
		m_pNextMood = pNewMood;
	return SetMood(pNewMood);
}

/* set the current default mood by name
*/
bool CMusicSystem::SetDefaultMood(const char *pszMood)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	m_sDefaultMood=pszMood;
	SMusicMood *pDefaultMood=GetDefaultMood(m_pCurrTheme);
	// force reselection of mood if we're currently playing the default mood
	if (m_fDefaultMoodTime<0.0f)
	{
		// we're currently playing the default mood so lets set the new default-mood
		m_fDefaultMoodTime=0.0f;
	}else
	{
		// we're currently playing some non-default mood, lets check if the new default mood-priority is higher than the currently playing one
		if (pDefaultMood && m_pCurrMood)
		{
			if (pDefaultMood->nPriority>m_pCurrMood->nPriority)
				m_fDefaultMoodTime=0.0f;	// default-mood priority is higher so lets choose it
		}
	}
	if (m_sDefaultMood.empty())
		LogMsg("[MusicSystem] Setting theme-specific default mood");
	else
		LogMsg("[MusicSystem] Setting default mood to %s", pszMood);
	return true;
}

/* get a vector of strings containing all available themes
*/
IStringItVec* CMusicSystem::GetThemes()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	TStringVec Vec;
	for (TThemeMapIt It=m_mapThemes.begin();It!=m_mapThemes.end();++It)
	{
		Vec.push_back(It->first.c_str());
	}
	return new CStringItVec(Vec);
}

/* get a vector of strings containing all available moods in the theme pszTheme
*/
IStringItVec* CMusicSystem::GetMoods(const char *pszTheme)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	TStringVec Vec;
	TThemeMapIt It=m_mapThemes.find(pszTheme);
	if (It==m_mapThemes.end())
		return NULL;
	SMusicTheme *pTheme=It->second;
	for (TMoodMapIt It=pTheme->mapMoods.begin();It!=pTheme->mapMoods.end();++It)
	{
		Vec.push_back(It->first.c_str());
	}
	return new CStringItVec(Vec);
}

/* add a mood-event
*/
bool CMusicSystem::AddMusicMoodEvent(const char *pszMood, float fTimeout)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	SMoodEventInfo EventInfo;
	EventInfo.sMood=pszMood;
	EventInfo.fTime=fTimeout;
	m_setFrameMoodEvents.insert(TMoodEventSetIt::value_type(EventInfo));
	return true;
}

/* remove all patterns from the play-lists
*/
void CMusicSystem::Silence()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	m_pCurrPattern=NULL;
	m_pNextPattern=NULL;
	m_nLayeredRhythmicPatterns=0;
	m_nLayeredIncidentalPatterns=0;
	m_vecPlayingPatterns.clear();
}

/* main update-loop; check comments in function for details
*/
void CMusicSystem::Update()
{
	FlushLog();
	if (!m_bDataLoaded)
		return;	// no data loaded

	if (m_pCVarMusicEnable->GetIVal() != m_musicEnable)
	{
		m_musicEnable = m_pCVarMusicEnable->GetIVal();
		if (!m_musicEnable)
		{
			Pause(true);
			Silence();
		}
		else
			Pause(false);
	}
	if (!m_musicEnable)
		return;

	// get deltatime
	float fDeltaTime=m_pTimer->GetFrameTime();
	// check if the volume has changed...
	if (m_pSoundSystem)
	{
		if (m_fMasterVolume!=m_pSoundSystem->GetMusicVolume())
		{
			GUARD_HEAP;
			m_fMasterVolume=m_pSoundSystem->GetMusicVolume();
			if (m_fMasterVolume>0.0f)
			{
				StartPlaying();	// start playing in case we're stopped (volume=0)
				CS_SetVolume(m_nChannel, (int)(m_fMasterVolume*255.0f));
			}else
			{
				StopPlaying();	// volume=0, lets stop playing the stream to reduce overhead
			}
		}
	}
	CSmartCriticalSection SmartCriticalSection(m_CS);
	// check for theme change
	if ((m_pCurrTheme!=m_pNextTheme) || (m_pThemeOverride))
	{
		SMusicTheme *pNext;
		if (m_pThemeOverride)
			pNext=m_pThemeOverride;
		else
			pNext=m_pNextTheme;
		m_bForcePatternChange=(m_pCurrTheme!=NULL) && (m_pCurrTheme!=pNext);	// try to get into the new theme asap if we're changing the theme
		if (m_pCurrTheme && pNext && m_bForcePatternChange)	// if we are already playing a theme we need to use a bridge
		{
			m_pNextPattern = ChooseBridge(m_pCurrTheme, pNext);
			if (!m_pNextPattern)
			{
				LogMsg("[MusicSystem] WARNING: Unable to find bridging pattern or no bridge defined from %s to %s !", m_pCurrTheme->sName.c_str(), m_pNextTheme->sName.c_str());
				// No bridge pattern, 
			}
		}
		m_pCurrTheme=pNext;
		if (m_pCurrTheme)
		{
			if (m_bForcePatternChange)
			{
				if (m_pNextMood)	// reenter next mood to get it from the correct theme
					SetMood( m_pNextMood );
				else
					m_pCurrMood=NULL;
			}
		}else
		{
			Silence();
		}
		if (!m_pCurrMood)
			EnterDefaultMood();	// if we are in no mood right now, use default mood
	}
	m_pNextMood = NULL;

	// event-handling
	if (m_setFrameMoodEvents.empty())
	{
		// no events occured this frame so lets delete the events-list
		if (!m_setMoodEvents.empty())
			m_setMoodEvents.clear();
	}
	// lets add all new events
	for (TMoodEventSetIt It=m_setFrameMoodEvents.begin();It!=m_setFrameMoodEvents.end();++It)
	{
		TMoodEventSetIt EventIt=m_setMoodEvents.find(TMoodEventSetIt::value_type(*It));
		if (EventIt==m_setMoodEvents.end())	// event not in list yet
			m_setMoodEvents.insert(TMoodEventSetIt::value_type(*It));
	}
	// lets not choose any mood which has a lower priority than the currently playing one...
	int nNewMoodPriority=-1;
	SMusicMood *pNewMood=NULL;
	for (TMoodEventSetIt It=m_setMoodEvents.begin();It!=m_setMoodEvents.end();)
	{
		// lets remove all events which didnt occur this frame...
		TMoodEventSetIt FrameIt=m_setFrameMoodEvents.find(TMoodEventSetIt::value_type(*It));
		if (FrameIt==m_setFrameMoodEvents.end())	// event not in list of last frame
		{
			// the event didn't occur, so we remove it here
			TMoodEventSetIt itnext = It;
			itnext++;
			m_setMoodEvents.erase(It);
			It = itnext;
		}else
		{
			// the event occured, so lets check if we reached the timeout
			SMoodEventInfo &EventInfo= const_cast<SMoodEventInfo&>(*It);
			EventInfo.fTime-=fDeltaTime;
			if (EventInfo.fTime<=0.0f)
			{
				// time to switch to this mood, so we store its priority so we can choose the one with the highest priority later
				SMusicMood *pMood=GetMood(m_pCurrTheme, EventInfo.sMood.c_str());
				if (pMood)
				{
					// does it have a higher priority than the one before ?
					if (pMood->nPriority>nNewMoodPriority)
					{
						// ...yes, store name and new priority
						nNewMoodPriority=pMood->nPriority;
						pNewMood=pMood;
					}
				}else
				{
					if (m_pCurrTheme)
						LogMsg("[MusicSystem] Invalid music mood '%s' (Theme: %s)", EventInfo.sMood.c_str(), m_pCurrTheme->sName.c_str());
					else
						LogMsg("[MusicSystem] Invalid music mood '%s' (Theme: NULL)", EventInfo.sMood.c_str());
				}
			}
			++It;
		}
	}
	m_setFrameMoodEvents.clear();
	// now we need to check if the current default mood has a higher priority than the current mood
	SMusicMood *pDefMood=GetDefaultMood(m_pCurrTheme);
	// select default mood, if...
	if (pDefMood																// ...there is a default mood
		&& (pDefMood->nPriority>nNewMoodPriority)	// ...it has a higher priority than the new mood
		&& (m_fDefaultMoodTime>=0.0f))						// ...we are not in the default mood already
	{
		// update default mood timing
		m_fDefaultMoodTime-=fDeltaTime;
		// is it time to enter the default mood ?
		if ((m_fDefaultMoodTime<0.0f) && (m_pCurrMood && (!m_pCurrMood->bPlaySingle)) && m_bEnableEventProcessing)
			EnterDefaultMood();					// ...yes
	}else
	{
		// one of the above criteria failed, so lets see if we can choose the new mood
		// do we have a new mood ?
		if (pNewMood)
		{
			// ...yes
			bool bEnterNewMood=true;
			// do we also have a default mood ?
			if (pDefMood && pNewMood)
			{
				// ...yes; does the default mood has a higher priority than the new one ?
				if (pDefMood->nPriority>pNewMood->nPriority)
					bEnterNewMood=false;							// ...yes, so we dont enter the new mood
			}
			if (m_pCurrMood && pNewMood)
			{
				// If currently playing mood is Play Single and have higher priority, doesnt switch to new mood.
				if (m_pCurrMood->bPlaySingle && m_pCurrMood->nPriority > pNewMood->nPriority)
					bEnterNewMood = false;
			}
			// should we enter the new mood ?
			if (bEnterNewMood && m_bEnableEventProcessing)
			{
				// ...yes
				SetMood(pNewMood);
				if (m_pCurrTheme)
					m_fDefaultMoodTime=m_pCurrTheme->fDefaultMoodTimeout;
			}
		}
	}
}

/* get system-status of music-system; returning pointer only valid until next call to GetStatus()
*/
SMusicSystemStatus* CMusicSystem::GetStatus()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	m_Status.m_vecPlayingPatterns.clear();
	m_Status.bPlaying=(m_nChannel!=-1);
	if (m_pCurrTheme)
		m_Status.sTheme=m_pCurrTheme->sName.c_str();
	else
		m_Status.sTheme="<none>";
	if (m_pCurrMood)
		m_Status.sMood=m_pCurrMood->sName.c_str();
	else
		m_Status.sMood="<none>";
	SPlayingPatternsStatus PatternStatus;
	// push current main-pattern
	if (m_pCurrPattern)
	{
		PatternStatus.nLayer=MUSICLAYER_MAIN;
		PatternStatus.sName=m_pCurrPattern->GetPattern()->GetName();
		PatternStatus.nVolume=255;
		m_Status.m_vecPlayingPatterns.push_back(PatternStatus);
	}
	for (TPatternPlayInfoVecIt It=m_vecPlayingPatterns.begin();It!=m_vecPlayingPatterns.end();++It)
	{
		SMusicPatternPlayInfo &PlayInfo=(*It);
		PatternStatus.nLayer=PlayInfo.nLayer;
		PatternStatus.sName=PlayInfo.pPatternInstance->GetPattern()->GetName();
		PatternStatus.nVolume=PlayInfo.pPatternInstance->GetPattern()->GetLayeringVolume();
		m_Status.m_vecPlayingPatterns.push_back(PatternStatus);
	}
	return &m_Status;
}

/* get pointer to mood-descriptor of current default mood (either theme specific or overriden default mood; whatever is currently set)
*/
SMusicMood* CMusicSystem::GetDefaultMood(SMusicTheme *pTheme)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!pTheme)
		return NULL;
	if (m_sDefaultMood.empty())
		return GetMood(pTheme, pTheme->sDefaultMood.c_str());
	else
	{
		SMusicMood *pMood=GetMood(pTheme, m_sDefaultMood.c_str());
		if (!pMood)
			return GetMood(pTheme, pTheme->sDefaultMood.c_str());		// return theme-specific mood if user defined default mood is not available
		else
			return pMood;
	}
}

/* enter the default mood (either theme specific or overriden default mood; whatever is currently set)
*/
void CMusicSystem::EnterDefaultMood()
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (m_pCurrTheme)
		SetMood(GetDefaultMood(m_pCurrTheme));
	else
		m_fDefaultMoodTime=0.0f;	// no theme is set yet, so lets try it again in the next update...
}

/* add a pattern to the play-list
*/
void CMusicSystem::PushPatternToMixList(SMusicPatternPlayInfo &PlayInfo)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	MTRACE("Pushing pattern %s in mix-list.", PlayInfo.pPatternInstance->GetPattern()->GetName());
	m_vecPlayingPatterns.push_back(PlayInfo);
}

/* main streaming callback
	this is the main state-machine; check comments in function for details
*/
signed char CMusicSystem::StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (m_pStream!=pStream)
	{
		// wrong stream; this should never happen
		CryError( "<CrySound> (CMusicSystem::StreamingCallback) Wrong Stream" );
		return FALSE;
	}
	// Ignore if not playing.
	if (!m_bPlaying)
		return TRUE;
	if (!m_musicEnable)
		return TRUE;

	signed short *pOutput=(signed short*)pBuffer;
	int nOfs=0;
	int nSamples=nLength/m_nBytesPerSample;
	// check if theme and mood are valid
	if ((!m_pCurrTheme) || (!m_pCurrMood) || (!UpdateCurrentPatternSet(m_pCurrMood, nSamples, false)))
	{
		// no theme or mood set
		if (m_pCurrPattern)
		{
			SMusicPatternPlayInfo PlayInfo;
			PlayInfo.nLayer=MUSICLAYER_MAIN;
			PlayInfo.pPatternInstance=m_pCurrPattern;
			PlayInfo.eBlendType=EBlend_FadeOut;
			PushPatternToMixList(PlayInfo);
			m_pCurrPattern=NULL;
		}
		memset(&(pOutput[nOfs]), 0, nSamples*m_nBytesPerSample);
		MixStreams(&(pOutput[nOfs]), nSamples);
		return TRUE;
	}
	// if no patterns are chosen, lets choose one
	if (!m_pCurrPattern)
		m_pCurrPattern=ChoosePattern(m_pCurrMood);
	if (!m_pNextPattern)
		m_pNextPattern=ChoosePattern(m_pCurrMood);
	for (;;)
	{
		if (!m_pCurrPattern)
		{
			// there is no pattern available... weired
			memset(&(pOutput[nOfs]), 0, nSamples*m_nBytesPerSample);
			break;
		}
		// retrieve samples to next fade-point so we can decide if we wanna play additional layers
		int nSamplesToNextFade=m_pCurrPattern->GetSamplesToNextFadePoint();	// fast
		// lets add some other layers
		if ((!m_bBridging) && (nSamplesToNextFade<=nSamples))	// are we not bridging and is the fade point in this block ?
		{
			// adjust streams ref-count, so it will free up patterns which will finish this frame
			AdjustMixStreamsRefCount(nSamplesToNextFade);
			// ...yes, lets eventually choose some layered patterns
			// rhythmic layer
			if ((m_nLayeredRhythmicPatterns < m_pCurrMood->pCurrPatternSet->nMaxSimultaneousRhythmicPatterns) &&
					(m_nLayeredRhythmicPatterns+m_nLayeredIncidentalPatterns) < m_pCVarMusicMaxSimultaniousPatterns->GetIVal())
			{
				// should we layer now ?
				bool bIsTimeForLayer=m_RandGen.Rand(0.0f, 100.0f)<m_pCurrMood->pCurrPatternSet->fRhythmicLayerProbability;
				if (bIsTimeForLayer)
				{
					// ...yes
					CMusicPatternInstance *pLayeredPattern=ChoosePattern(m_pCurrMood, MUSICLAYER_RHYTHMIC);
					if (pLayeredPattern)
					{
						m_nLayeredRhythmicPatterns++;
						pLayeredPattern->Seek0(nSamplesToNextFade);	// lets seek negative so it starts playing at the right time...
						// lets queue the new pattern so it gets automatically played till the end
						SMusicPatternPlayInfo PlayInfo;
						PlayInfo.nLayer=MUSICLAYER_RHYTHMIC;
						PlayInfo.pPatternInstance=pLayeredPattern;
						PlayInfo.eBlendType=EBlend_ToEnd;
						PlayInfo.pRefCount=&m_nLayeredRhythmicPatterns;
						PushPatternToMixList(PlayInfo);
	#ifdef _DEBUG
						SMusicPatternFileInfo FileInfo;
						pLayeredPattern->GetPattern()->GetFileInfo(FileInfo);
						MTRACE("Layering rhythmic-layer %s", FileInfo.sFilename.c_str());
	#endif
					}
				}
			}
			// incidental layer
			if ((m_nLayeredIncidentalPatterns < m_pCurrMood->pCurrPatternSet->nMaxSimultaneousIncidentalPatterns) &&
					(m_nLayeredRhythmicPatterns+m_nLayeredIncidentalPatterns) < m_pCVarMusicMaxSimultaniousPatterns->GetIVal())
			{
				// should we layer now ?
				bool bIsTimeForLayer=m_RandGen.Rand(0.0f, 100.0f)<m_pCurrMood->pCurrPatternSet->fIncidentalLayerProbability;
				if (bIsTimeForLayer)
				{
					// ...yes
					CMusicPatternInstance *pLayeredPattern=ChoosePattern(m_pCurrMood, MUSICLAYER_INCIDENTAL);
					if (pLayeredPattern)
					{
						m_nLayeredIncidentalPatterns++;
						pLayeredPattern->Seek0(nSamplesToNextFade);	// lets seek negative so it starts playing at the right time...
						// lets queue the new pattern so it gets automatically played till the end
						SMusicPatternPlayInfo PlayInfo;
						PlayInfo.nLayer=MUSICLAYER_INCIDENTAL;
						PlayInfo.pPatternInstance=pLayeredPattern;
						PlayInfo.eBlendType=EBlend_ToEnd;
						PlayInfo.pRefCount=&m_nLayeredIncidentalPatterns;
						PushPatternToMixList(PlayInfo);
	#ifdef _DEBUG
						SMusicPatternFileInfo FileInfo;
						pLayeredPattern->GetPattern()->GetFileInfo(FileInfo);
						MTRACE("Layering incidental-layer %s", FileInfo.sFilename.c_str());
	#endif
					}
				}
			}
		}
		int nSamplesToFade;	// samples to main-pattern change
		// slow or fast fade ?
		if (m_bForcePatternChange)
			nSamplesToFade=nSamplesToNextFade;	// fast
		else
			nSamplesToFade=m_pCurrPattern->GetSamplesToEnd(); //GetSamplesToLastFadePoint();	// slow
		if (nSamplesToFade<=nSamples)	// is the fade-point in this block ?
		{
			// ...yes
			m_pCurrPattern->GetPCMData((signed long*)&(pOutput[nOfs]), nSamplesToFade);	// get pcm data from the current pattern, till we reach the fadepoint
			MixStreams(&(pOutput[nOfs]), nSamplesToFade);	// also mix all queued patterns to the fadepoint
			// if the old stream is not at the end yet we push it in the PlayingStreams-list so it gets played and faded...
//			if (m_pCurrPattern->GetSamplesToEnd()>0)
			if (m_bForcePatternChange && (!m_bCurrPatternIsBridge))
			{
				SMusicPatternPlayInfo PlayInfo;
				PlayInfo.nLayer=MUSICLAYER_MAIN;
				PlayInfo.pPatternInstance=m_pCurrPattern;
				PlayInfo.eBlendType=EBlend_FadeOut;
				PushPatternToMixList(PlayInfo);
			}else
			{
				// if this is a single-play mood we enter the default mood now, since we're done with the pattern
				if (m_pCurrMood->bPlaySingle)
					EnterDefaultMood();
			}
			if (m_bBridging)
			{
				m_bCurrPatternIsBridge=true;
				MTRACE("Clearing bridging-flag.");
			}else
			{
				m_bCurrPatternIsBridge=false;
			}
			m_bBridging=false;
			UpdateCurrentPatternSet(m_pCurrMood, 0, true);
			// current<=next
			m_pCurrPattern=m_pNextPattern;
			// lets choose a new "next" pattern
			m_pNextPattern=ChoosePattern(m_pCurrMood);
			// we should always have a new current pattern at this point, but lets check in case there were non available
			if (m_pCurrPattern)
			{
				m_pCurrPattern->Seek0();		// seek to 0
#ifdef _DEBUG
				SMusicPatternFileInfo FileInfo;
				m_pCurrPattern->GetPattern()->GetFileInfo(FileInfo);
				MTRACE("Changing Music-Pattern to %s", FileInfo.sFilename.c_str());
#endif
			}
			// enhance positions
			nOfs+=nSamplesToFade<<1;
			nSamples-=nSamplesToFade;
			// update flags
			m_bForcePatternChange=false;
		}else
		{
			// no fadepoint in this block so we can use the current patterns without any hassle
			m_pCurrPattern->GetPCMData((signed long*)&(pOutput[nOfs]), nSamples);	// get pcm data
			MixStreams(&(pOutput[nOfs]), nSamples);	// mix queued patterns
			break;
		}
	}
	return TRUE;
}

/* lookahead for all patterns in play-list to see which ones exceed in the next nSamples
*/
void CMusicSystem::AdjustMixStreamsRefCount(int nSamples)
{
	for (TPatternPlayInfoVecIt It=m_vecPlayingPatterns.begin();It!=m_vecPlayingPatterns.end();++It)
	{
		SMusicPatternPlayInfo &PlayInfo=(*It);
		if (!PlayInfo.pRefCount)
			continue;
		int nSamplesToEnd=PlayInfo.pPatternInstance->GetSamplesToEnd();
		if (nSamplesToEnd<=nSamples)
		{
			(*(PlayInfo.pRefCount))--;
			PlayInfo.bRefCountAdjusted=true;
		}
	}
}

/* mix all streams in the play-list into pBuffer; mixlength is nSamples
	the data is processed before mixing if needed (eg. ramping)
*/
void CMusicSystem::MixStreams(void *pBuffer, int nSamples)
{
	for (TPatternPlayInfoVecIt It=m_vecPlayingPatterns.begin();It!=m_vecPlayingPatterns.end();)
	{
		int nSamplesToRead=nSamples;
		SMusicPatternPlayInfo &PlayInfo=(*It);
		bool bStreamEnd=false;
		if (PlayInfo.eBlendType==EBlend_Stop)
		{
			bStreamEnd=true;
		}else
		{
			GUARD_HEAP; // AMD64 note: here was a Memory Corruption
			switch (PlayInfo.eBlendType)
			{
				case EBlend_ToEnd:
				{
					int nLength=PlayInfo.pPatternInstance->GetSamplesToEnd();
					if (nSamplesToRead>=nLength)
					{
						nSamplesToRead=nLength;
						bStreamEnd=true;
					}
					break;
				}
			}
			PlayInfo.pPatternInstance->GetPCMData((signed long*)m_pMixBuffer, nSamplesToRead);
      
			switch (PlayInfo.eBlendType)
			{
				case EBlend_FadeOut:
				{
					if (FadeStream(PlayInfo, (signed short*)m_pMixBuffer, nSamplesToRead))
						bStreamEnd=true;
					break;
				}
			}
			//const int nSize = 1024*1024*4;
			//static int arrBuffer[nSize];
			//memset (arrBuffer, 0xCE, sizeof(arrBuffer));
      //memcpy (pBuffer, m_pMixBuffer, nSamplesToRead*4);
      //memset(pBuffer, 0, nSamplesToRead*4);
			CS_DSP_MixBuffers(pBuffer, m_pMixBuffer, nSamplesToRead, m_nSampleRate, PlayInfo.pPatternInstance->GetPattern()->GetLayeringVolume(), 128, CS_16BITS | CS_STEREO);
/*
  signed short *destptr = (signed short *)pBuffer;
	signed int *srcptr = (signed int *)pBuffer;

	for (int count = 0; count < nSamplesToRead; count++)
	{
		signed int lval,rval;
		lval = *srcptr++;
		rval = *srcptr++;

		*destptr++ = (signed short)( lval < -32768 ? -32768 : lval > 32767 ? 32767 : lval);
		*destptr++ = (signed short)( rval < -32768 ? -32768 : rval > 32767 ? 32767 : rval);
  }
/**/
  
     /*assert (arrBuffer[nSize-1] == 0xCECECECE);
			assert (IsHeapValid());
			memcpy (pBuffer, arrBuffer, nSamplesToRead*4);
			assert (IsHeapValid());*/
		}
		if (bStreamEnd)
		{
			if (PlayInfo.pRefCount && (!PlayInfo.bRefCountAdjusted))
				(*(PlayInfo.pRefCount))--;
			It=m_vecPlayingPatterns.erase(It);
		}else
			++It;
	}
}

/* volume ramping for a pattern
*/
bool CMusicSystem::FadeStream(SMusicPatternPlayInfo &PlayInfo, signed short *pBuffer, int nSamples)
{
	double fFadeTime=PlayInfo.fFadeTime;
	if (!fFadeTime)
		fFadeTime=m_fCurrCrossfadeTime;
	double fPhaseAdder=(1.0/(double)m_nSampleRate)/fFadeTime;
	for (int i=0;i<nSamples;i++)
	{
		if (PlayInfo.fPhase<=0.0)
		{
			memset(pBuffer, 0, (nSamples-i)*m_nBytesPerSample);
			PlayInfo.fPhase=0.0;
			return true;
		}
		*pBuffer=(signed short)((float)(*pBuffer)*PlayInfo.fPhase);
		pBuffer++;
		*pBuffer=(signed short)((float)(*pBuffer)*PlayInfo.fPhase);
		pBuffer++;
		PlayInfo.fPhase-=fPhaseAdder;
	}
	return false;
}

/* all secondary layers (rhythmic, incidental) will be faded out over fFadeTime seconds
*/
void CMusicSystem::FadeOutAllSecondaryLayers(double fFadeTime)
{
	for (TPatternPlayInfoVecIt It=m_vecPlayingPatterns.begin();It!=m_vecPlayingPatterns.end();++It)
	{
		SMusicPatternPlayInfo &PlayInfo=(*It);
		if (PlayInfo.eBlendType==EBlend_Stop)
			continue;
		PlayInfo.eBlendType=EBlend_FadeOut;
		PlayInfo.fFadeTime=fFadeTime;
	}
}

/* create a new instance of pattern pszPattern
*/
CMusicPatternInstance* CMusicSystem::GetPatternInstance(const char *pszPattern)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	TPatternMapIt PatternIt=m_mapPatterns.find(pszPattern);
	if (PatternIt==m_mapPatterns.end())
	{
		LogMsg("[MusicSystem] WARNING: Pattern %s could not be found !", pszPattern);
		return NULL;
	}
	CMusicPattern *pPattern=PatternIt->second;
	return pPattern->CreateInstance();
}

/* check if a new pattern-set needs to be chosen
*/
bool CMusicSystem::UpdateCurrentPatternSet(SMusicMood *pMood, int nSamples, bool bAllowChange)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!pMood)
		return false;
	bool bChooseNewPatternSet=(pMood->pCurrPatternSet==NULL);
	pMood->fCurrPatternSetTime+=(float)nSamples/(float)m_nSampleRate;	// update timing
	if (bAllowChange)
	{
		if (pMood->fCurrPatternSetTime>pMood->fCurrPatternSetTimeout)	// did we reach the timeout ?
			bChooseNewPatternSet=true;	// yes... lets choose a new pattern-set
		if (bChooseNewPatternSet)
		{
			ChoosePatternSet(pMood);
			return (pMood->pCurrPatternSet!=NULL);
		}
	}
	return true;
}

/* choose a new pattern-set and calc its timeout
*/
bool CMusicSystem::ChoosePatternSet(SMusicMood *pMood)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!pMood)
		return false;
	SMusicPatternSet *pPrevPatternSet=pMood->pCurrPatternSet;
	int nPatternSets= (int)pMood->vecPatternSets.size();
	SMusicPatternSet *pPatternSet=NULL;
	int nCount=0;
	switch (nPatternSets)
	{
		case 0:
			LogMsg("[MusicSystem] WARNING: Choosing NULL pattern-set.");
			return false;
		case 1:
			pPatternSet=*(pMood->vecPatternSets.begin());
			break;
		default:
			int nPatternSet=(int)m_RandGen.Rand(0.0f, (float)nPatternSets) % nPatternSets;
			for (TPatternSetVecIt It=pMood->vecPatternSets.begin();It!=pMood->vecPatternSets.end();++It)
			{
				if (nCount==nPatternSet)
				{
					pPatternSet=(*It);
					while (pPatternSet==pPrevPatternSet)	// make sure we select a different set
					{
						++It;
						nCount++;
						if (It==pMood->vecPatternSets.end())
						{
							It=pMood->vecPatternSets.begin();
							nCount=0;
						}
						pPatternSet=(*It);
					}
					break;
				}
				nCount++;
			}
			//FadeOutAllSecondaryLayers(PATTERNSET_CHANGE_FADETIME);
			break;
	}
	pMood->fCurrPatternSetTime=0.0f;
	pMood->fCurrPatternSetTimeout=m_RandGen.Rand(pPatternSet->fMinTimeout, pPatternSet->fMaxTimeout);
	LogMsg("[MusicSystem] Choosing pattern-set %d with a timeout of %3.1f seconds.", nCount, pMood->fCurrPatternSetTimeout);
	pMood->pCurrPatternSet=pPatternSet;
	// we have to choose a new next pattern in order to reflect the new pattern set
	m_pNextPattern=ChoosePattern(pMood);
	return true;
}

/* return bridge from pCurrTheme to pNewTheme; NULL if no such bridge exist
*/
CMusicPatternInstance* CMusicSystem::ChooseBridge(SMusicTheme *pCurrTheme, SMusicTheme *pNewTheme)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if ((!pCurrTheme) || (!pNewTheme))
		return NULL;
	TThemeBridgeMapIt BridgeIt=pCurrTheme->mapBridges.find(pNewTheme->sName);
	if (BridgeIt==pCurrTheme->mapBridges.end())
		return NULL;	// no bridge defined
	CMusicPatternInstance *pBridgePattern=GetPatternInstance(BridgeIt->second.c_str());
	if (pBridgePattern)
		m_bBridging=true;	// set bridging flag
	if (m_bBridging)
		MTRACE("Setting bridging-flag.");
	return pBridgePattern;
}

/* choose a pattern and return its instance for pMood in nLayer
*/
CMusicPatternInstance* CMusicSystem::ChoosePattern(SMusicMood *pMood, int nLayer)
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	if (!pMood)
		return NULL;
	if (m_mapPatterns.empty())
		return NULL;
	TPatternDefVec *pPatternInfo=NULL;
	float fTotalProbability;
	switch (nLayer)
	{
		default:
		case MUSICLAYER_MAIN:
			if (m_bBridging)
			{
				MTRACE("Returning bridging pattern blindly...");
				return m_pNextPattern;	// if we're bridging we return the "NextPattern" since we want to continue the bridging
			}
			pPatternInfo=&(pMood->pCurrPatternSet->vecMainPatterns);
			fTotalProbability=pMood->pCurrPatternSet->fTotalMainPatternProbability;
			break;
		case MUSICLAYER_RHYTHMIC:
			pPatternInfo=&(pMood->pCurrPatternSet->vecRhythmicPatterns);
			fTotalProbability=pMood->pCurrPatternSet->fTotalRhythmicPatternProbability;
			break;
		case MUSICLAYER_INCIDENTAL:
			pPatternInfo=&(pMood->pCurrPatternSet->vecIncidentalPatterns);
			fTotalProbability=pMood->pCurrPatternSet->fTotalIncidentalPatternProbability;
			break;
	}
	if ((!pPatternInfo) || (pPatternInfo->empty()))
		return NULL;	// no pattern could be found in the requested layer
	// lets choose one accourding to their probability
	float fProb = fTotalProbability;
	float fRand=m_RandGen.Rand(0.0f, fTotalProbability);
	fTotalProbability=0.0f;
	
	const char *sPattern = "";
	// iterate through patterns and retrieve the one which was chosen by fTotalProbability
	TPatternDefVecIt InfoIt;
	for (InfoIt=pPatternInfo->begin();InfoIt!=pPatternInfo->end();++InfoIt)
	{
		SPatternDef *pPattern = (*InfoIt);
		fTotalProbability+=pPattern->fProbability;
		if (fRand<=fTotalProbability)
		{
			sPattern = pPattern->sName.c_str();
			break;
		}
	}
	ASSERT( strlen(sPattern) > 0 );
	if (nLayer==MUSICLAYER_INCIDENTAL)	// make sure we dont choose a pattern which is already played
	{
		bool bPatternFound;
		int nTries=0;
		do
		{
			bPatternFound=true;
			for (TPatternPlayInfoVecIt It=m_vecPlayingPatterns.begin();It!=m_vecPlayingPatterns.end();++It)
			{
				SMusicPatternPlayInfo &Info=(*It);
				if (stricmp(Info.pPatternInstance->GetPattern()->GetName(),sPattern )==0)	// in use already
				{
					bPatternFound=false;
					break;
				}
			}
			if (!bPatternFound)
			{
				InfoIt++;
				if (InfoIt==pPatternInfo->end())
					InfoIt=pPatternInfo->begin();
				SPatternDef *pPattern = (*InfoIt);
				sPattern = pPattern->sName.c_str();
				nTries++;
				if (nTries==(int)pPatternInfo->size())
					return NULL;	// nothing found
			}
		}while(!bPatternFound);
	}
	return GetPatternInstance( sPattern );
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::GetMemoryUsage(class ICrySizer* pSizer)
{
	if (!pSizer->Add(*this))
		return;
	if (m_pMixBuffer)
		pSizer->AddObject(m_pMixBuffer, (int)((float)m_nSampleRate*m_fLatency)*m_nBytesPerSample);
	{
		SIZER_COMPONENT_NAME(pSizer, "Patterns");
		for (TPatternMapIt PatternIt=m_mapPatterns.begin();PatternIt!=m_mapPatterns.end();++PatternIt)
		{
			CMusicPattern *pPattern=PatternIt->second;
			pPattern->GetMemoryUsage(pSizer);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::RenamePattern( const char *sOldName,const char *sNewName )
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	CMusicPattern *pPattern = stl::find_in_map( m_mapPatterns,sOldName,(CMusicPattern *)NULL);
	if (pPattern)
	{
		m_mapPatterns.erase(sOldName);
		m_mapPatterns[sNewName] = pPattern;
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::UpdateTheme( SMusicTheme *pTheme )
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	Silence();
	m_pCurrTheme = m_pNextTheme = m_pThemeOverride = 0;
	m_pCurrMood = 0;
	m_mapThemes = m_pDataPtr->mapThemes;
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::UpdateMood( SMusicMood *pMood )
{
	m_pCurrMood = 0;
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::UpdatePattern( SPatternDef *pPatternDef )
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	// Update pattern.
	CMusicPattern *pPattern = stl::find_in_map( m_mapPatterns,pPatternDef->sName.c_str(), (CMusicPattern *)NULL);
	if (pPattern)
	{
		bool bStopped = false;
		// Update exising pattern.
		if (stricmp(pPatternDef->sFilename.c_str(),pPattern->GetFilename()) != 0)
		{
			Silence();
			bStopped = StopPlaying();

			pPattern->SetFilename( pPatternDef->sFilename.c_str() );
			/*
			// Filename changed.
			const char *pszFilename = pPatternDef->sFilename.c_str();
			if (!pPattern->Open(pszFilename))
			{
				delete pPattern;
				LogMsg("[MusicSystem] WARNING: Cannot load music-pattern %s (%s) !", pPatternDef->sName.c_str(), pszFilename);
				return;
			}
			*/
		}
		pPattern->ClearFadePoints();
		for (TIntVecIt IntIt=pPatternDef->vecFadePoints.begin();IntIt!=pPatternDef->vecFadePoints.end();++IntIt)
		{
			pPattern->AddFadePoint(*IntIt);
		}
		pPattern->SetLayeringVolume(pPatternDef->nLayeringVolume);
		if (bStopped)
			StartPlaying();
	}
	else
	{
		// Add this pattern.
		CMusicPattern *pPattern = AddPattern(pPatternDef->sName.c_str(),pPatternDef->sFilename.c_str());
		if (pPattern)
		{
			for (TIntVecIt IntIt=pPatternDef->vecFadePoints.begin();IntIt!=pPatternDef->vecFadePoints.end();++IntIt)
			{
				pPattern->AddFadePoint(*IntIt);
			}
			pPattern->SetLayeringVolume(pPatternDef->nLayeringVolume);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::PlayPattern( const char *sPattern,bool bStopPrevious )
{
	CSmartCriticalSection SmartCriticalSection(m_CS);
	CMusicPattern *pPattern = stl::find_in_map( m_mapPatterns,sPattern,(CMusicPattern *)NULL);
	if (pPattern)
	{
		m_bForcePatternChange = true;
		if (bStopPrevious)
		{
			Silence();
			m_pCurrTheme = m_pNextTheme = m_pThemeOverride = 0;
			m_pCurrMood = 0;
			m_pCurrPattern = pPattern->CreateInstance();
			m_pNextPattern = m_pCurrPattern;
		}
		else
		{
			CMusicPatternInstance *pLayeredPattern = pPattern->CreateInstance();
			if (pLayeredPattern)
			{
				m_nLayeredIncidentalPatterns++;
				//pLayeredPattern->Seek0(nSamplesToNextFade);	// lets seek negative so it starts playing at the right time...
				// lets queue the new pattern so it gets automatically played till the end
				SMusicPatternPlayInfo PlayInfo;
				PlayInfo.nLayer=MUSICLAYER_INCIDENTAL;
				PlayInfo.pPatternInstance=pLayeredPattern;
				PlayInfo.eBlendType=EBlend_ToEnd;
				PlayInfo.pRefCount=&m_nLayeredIncidentalPatterns;
				PushPatternToMixList(PlayInfo);
			}
			m_pNextPattern = pPattern->CreateInstance();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::DeletePattern( const char *sPattern )
{
	m_mapPatterns.erase( sPattern );
}


//////////////////////////////////////////////////////////////////////////
void CMusicSystem::LoadPatternFromXML( XmlNodeRef node,SPatternDef *pPattern )
{
	// Loading.
	pPattern->sName = node->getAttr( "Name" );
	node->getAttr( "Probability",pPattern->fProbability );
	node->getAttr( "LayeringVolume",pPattern->nLayeringVolume );
	pPattern->sFilename = node->getAttr( "Filename" );

	const char *sFadePoints = node->getAttr( "FadePoints" );
	if (strlen(sFadePoints) > 0)
	{
		char sPoints[4096];
		strncpy( sPoints,sFadePoints,sizeof(sPoints) );
		sPoints[sizeof(sPoints)-1] = '\0';

		char *token = strtok( sPoints,"," );
		while( token != NULL )
		{
			pPattern->vecFadePoints.push_back( atoi(token) );
			token = strtok( NULL,"," );
		}
	}
	// Add this pattern to music data.
	m_pDataPtr->vecPatternDef.push_back(pPattern);
	UpdatePattern( pPattern );
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::LoadMoodFromXML( XmlNodeRef &node,SMusicMood *pMood )
{
	pMood->sName = node->getAttr( "Name" );
	node->getAttr( "PlaySingle",pMood->bPlaySingle );
	node->getAttr( "Priority",pMood->nPriority );
	node->getAttr( "FadeOutTime",pMood->fFadeOutTime );
	if (node->getChildCount() > 0)
	{
		// Save pattern sets.
		for (int i = 0; i < node->getChildCount(); i++)
		{
			XmlNodeRef nodePtrnSet = node->getChild(i);

			SMusicPatternSet *pPatternSet = new SMusicPatternSet;
			pMood->vecPatternSets.push_back(pPatternSet);

			nodePtrnSet->getAttr( "MaxTimeout",pPatternSet->fMaxTimeout );
			nodePtrnSet->getAttr( "MinTimeout",pPatternSet->fMinTimeout );
			nodePtrnSet->getAttr( "IncidentalLayerProbability",pPatternSet->fIncidentalLayerProbability );
			nodePtrnSet->getAttr( "RhythmicLayerProbability",pPatternSet->fRhythmicLayerProbability );
			nodePtrnSet->getAttr( "MaxSimultaneousIncidentalPatterns",pPatternSet->nMaxSimultaneousIncidentalPatterns );
			nodePtrnSet->getAttr( "MaxSimultaneousRhythmicPatterns",pPatternSet->nMaxSimultaneousRhythmicPatterns );

			// Save patterns.
			XmlNodeRef nodeMainLayer = nodePtrnSet->findChild( "MainLayer" );
			XmlNodeRef nodeRhythmicLayer = nodePtrnSet->findChild( "RhythmicLayer" );
			XmlNodeRef nodeIncidentalLayer = nodePtrnSet->findChild( "IncidentalLayer" );

			if (nodeMainLayer)
			{
				for (int j = 0; j < nodeMainLayer->getChildCount(); j++)
				{
					XmlNodeRef childNode = nodeMainLayer->getChild(j);
					SPatternDef *pPattern = new SPatternDef;
					pPatternSet->vecMainPatterns.push_back(pPattern);
					LoadPatternFromXML( childNode,pPattern );
				}
			}
			if (nodeRhythmicLayer)
			{
				for (int j = 0; j < nodeRhythmicLayer->getChildCount(); j++)
				{
					XmlNodeRef childNode = nodeRhythmicLayer->getChild(j);
					SPatternDef *pPattern = new SPatternDef;
					pPatternSet->vecRhythmicPatterns.push_back(pPattern);
					LoadPatternFromXML( childNode,pPattern );
				}
			}
			if (nodeIncidentalLayer)
			{
				for (int j = 0; j < nodeIncidentalLayer->getChildCount(); j++)
				{
					XmlNodeRef childNode = nodeIncidentalLayer->getChild(j);
					SPatternDef *pPattern = new SPatternDef;
					pPatternSet->vecIncidentalPatterns.push_back(pPattern);
					LoadPatternFromXML( childNode,pPattern );
				}
			}

			//////////////////////////////////////////////////////////////////////////
			pPatternSet->fTotalMainPatternProbability = 0.0f;
			pPatternSet->fTotalRhythmicPatternProbability = 0.0f;
			pPatternSet->fTotalIncidentalPatternProbability = 0.0f;
			for (int j = 0; j < (int)pPatternSet->vecMainPatterns.size(); j++)
			{
				pPatternSet->fTotalMainPatternProbability += pPatternSet->vecMainPatterns[j]->fProbability;
			}
			for (int j = 0; j < (int)pPatternSet->vecRhythmicPatterns.size(); j++)
			{
				pPatternSet->fTotalRhythmicPatternProbability += pPatternSet->vecRhythmicPatterns[j]->fProbability;
			}
			for (int j = 0; j < (int)pPatternSet->vecIncidentalPatterns.size(); j++)
			{
				pPatternSet->fTotalIncidentalPatternProbability += pPatternSet->vecIncidentalPatterns[j]->fProbability;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMusicSystem::LoadThemeFromXML( XmlNodeRef &node,SMusicTheme *pTheme )
{
	// Loading.
	pTheme->sName = node->getAttr( "Name" );
	pTheme->sDefaultMood = node->getAttr( "DefaultMood" );
	node->getAttr( "DefaultMoodTimeout",pTheme->fDefaultMoodTimeout );

	XmlNodeRef nodeMoods = node->findChild("Moods");
	if (nodeMoods)
	{
		for (int i = 0; i < nodeMoods->getChildCount(); i++)
		{
			XmlNodeRef childNode = nodeMoods->getChild(i);
			SMusicMood *pMood = new SMusicMood;
			LoadMoodFromXML( childNode,pMood );
			pTheme->mapMoods[pMood->sName.c_str()] = pMood;
		}
	}

	XmlNodeRef nodeBridges = node->findChild( "Bridges" );
	if (nodeBridges)
	{
		for (int i = 0; i < nodeBridges->getChildCount(); i++)
		{
			XmlNodeRef nodeBridge = nodeBridges->getChild(i);
			const char *sPattern = nodeBridge->getAttr( "Pattern" );
			if (strlen(sPattern) > 0)
			{
				pTheme->mapBridges[nodeBridges->getTag()] = (const char*)sPattern;
			}
		}
	}

	// Add this pattern to music data.
	m_pDataPtr->mapThemes[pTheme->sName.c_str()] = pTheme;
}

//////////////////////////////////////////////////////////////////////////
bool CMusicSystem::LoadFromXML( const char *sFilename,bool bAddData )
{
	CSmartCriticalSection SmartCriticalSection(m_CS);

	// Loading.
	if (!bAddData)
	{
		Unload();
		m_pDataPtr = new SMusicData;
	}

	m_bOwnMusicData = true;
	if (!m_pDataPtr)
		m_pDataPtr = new SMusicData;

	XmlNodeRef root = m_pSystem->LoadXmlFile( sFilename );
	if (!root)
	{
		m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,sFilename,
			"Music description XML file %s failed to load",sFilename );
		return false;
	}

	// Iterate music themes
	for (int i = 0; i < root->getChildCount(); i++)
	{
		XmlNodeRef nodeTheme = root->getChild(i);
		if (nodeTheme->isTag("Theme"))
		{
			SMusicTheme *pTheme = new SMusicTheme;
			LoadThemeFromXML( nodeTheme,pTheme );
		}
	}

	m_mapThemes = m_pDataPtr->mapThemes;

	m_bDataLoaded = true;
	// start playing the stream...
	if (m_nChannel<0)
		StartPlaying();

	return true;
}

bool 
CMusicSystem::StreamOGG()
{
	return( 1 == m_pCVarMusicStreamedData->GetIVal() );
}
