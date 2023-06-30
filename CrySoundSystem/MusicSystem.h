#pragma once

#include <vector>
#include <isound.h>
#include <itimer.h>
#include <SmartPtr.h>
#include "MusicPattern.h"
#include "RandGen.h"

//#define TRACE_MUSIC

#ifdef TRACE_MUSIC
#define MTRACE TRACE
#else
#define MTRACE __noop
#endif

#define PATTERNSET_CHANGE_FADETIME	2.0

// Forward declarations
struct ISystem;

// Smart Pattern-Ptr
typedef _smart_ptr<CMusicPatternInstance>			TPatternInstancePtr;

// Type of blending
enum EBlendingType
{
	EBlend_FadeOut,
	EBlend_ToEnd,
	EBlend_Stop
};

// Info-Structure for automated fading
struct SMusicPatternPlayInfo
{
	SMusicPatternPlayInfo()
	{
		nLayer=MUSICLAYER_MAIN;
		pPatternInstance=NULL;
		eBlendType=EBlend_ToEnd;
		pRefCount=NULL;
		fPhase=1.0;
		fFadeTime=0.0;	// use default fade time...
		bRefCountAdjusted=false;
	}
	~SMusicPatternPlayInfo()
	{
		pPatternInstance=NULL;
	}
	TPatternInstancePtr pPatternInstance;
	int nLayer;
	EBlendingType eBlendType;
	int *pRefCount;
	// internal
	double fPhase;	// phase for fading (0-1)
	double fFadeTime;	// time in seconds to fade
	bool bRefCountAdjusted;
};

//////////////////////////////////////////////////////////////////////////
// A couple of typedefs for various stl-containers
//////////////////////////////////////////////////////////////////////////

typedef std::map<string,CMusicPattern*,stl::less_stricmp<string> >	TPatternMap;
typedef TPatternMap::iterator									TPatternMapIt;

typedef std::vector<string>							TStringVec;
typedef TStringVec::iterator									TStringVecIt;

typedef std::vector<SMusicPatternPlayInfo>		TPatternPlayInfoVec;
typedef TPatternPlayInfoVec::iterator					TPatternPlayInfoVecIt;

//////////////////////////////////////////////////////////////////////////
// Structure to collect/handle mood-events
//////////////////////////////////////////////////////////////////////////

struct SMoodEventInfo
{
	bool operator<(const SMoodEventInfo &b) const
	{
		if (stricmp(sMood.c_str(),b.sMood.c_str()) < 0)
			return true;
		return false;
	}
	string sMood;// name of mood
	float fTime;			// time of consecutive event-occurance
};

typedef std::set<SMoodEventInfo>	TMoodEventSet;
typedef TMoodEventSet::iterator		TMoodEventSetIt;

//////////////////////////////////////////////////////////////////////////
// The MusicSystem itself
//////////////////////////////////////////////////////////////////////////

class CMusicSystem : public IMusicSystem
{
protected:
	CRITICAL_SECTION m_CS;
	// system pointers
	ISystem *m_pSystem;
	ITimer *m_pTimer;
	ILog *m_pLog;
	ISoundSystem *m_pSoundSystem;
	// random generator
	CPseudoRandGen m_RandGen;
	// notification sink
	IMusicSystemSink *m_pSink;
	// current data (allocated externally)
	SMusicData *m_pDataPtr;
	bool m_bOwnMusicData;
	// sound-playback params
	float m_fMasterVolume;
	int m_nSampleRate;
	float m_fLatency;
	// bytes/sample in output stream
	int m_nBytesPerSample;
	// fmod stream data
	CS_STREAM *m_pStream;
	int m_nChannel;
	// states
	bool m_bDataLoaded;
	bool m_bPause;
	bool m_bPlaying;
	bool m_bBridging;						// bridging in progress
	bool m_bCurrPatternIsBridge;
	bool m_bForcePatternChange;	// change pattern asap (next fadepoint), do not play to the end (last fadepoint)
	// status
	SMusicSystemStatus m_Status;
	// all patterns
	TPatternMap m_mapPatterns;
	// current patterns
	TPatternInstancePtr m_pCurrPattern;
	TPatternInstancePtr m_pNextPattern;
	int m_nLayeredRhythmicPatterns;
	int m_nLayeredIncidentalPatterns;
	// temporary mix-buffer
	void *m_pMixBuffer;
	// array of currently playing patterns
	TPatternPlayInfoVec m_vecPlayingPatterns;
	// all themes (moods inside)
	TThemeMap m_mapThemes;
	// current theme/mood
	SMusicTheme *m_pThemeOverride;				// overriding theme (eg. for vehicle)
	SMusicTheme *m_pCurrTheme;
	SMusicTheme *m_pNextTheme;						// in case a theme change occured, this is the new theme... NULL otherwise
	SMusicMood *m_pCurrMood;
	SMusicMood *m_pNextMood;
	// mood event arrays
	TMoodEventSet m_setMoodEvents;				// active mood events
	TMoodEventSet m_setFrameMoodEvents;		// mood events arrived last frame
	// default mood
//SMusicMood *m_pDefaultMood;
	float m_fDefaultMoodTime;	// current time 'till default-mood-timeout
	string m_sDefaultMood;	// override of theme-specific default-mood (set by SetDefaultMood); set to "" to use theme-specific
	// fading
	double m_fCurrCrossfadeTime;
	// logging
	TStringVec m_vecLog;
	bool m_bEnableEventProcessing;
	ICVar	*m_pCVarDebugMusic;
	ICVar	*m_pCVarMusicEnable;
	ICVar	*m_pCVarMusicMaxSimultaniousPatterns;
	ICVar	*m_pCVarMusicStreamedData;
	int m_musicEnable;
protected:
	virtual ~CMusicSystem();
	bool Init();
	void Shutdown();
	void FlushLog();
	CMusicPattern* AddPattern(const char *pszName, const char *pszFilename);
	bool FlushPatterns();
	bool StartPlaying();
	bool StopPlaying();
	SMusicMood* GetMood(SMusicTheme *pTheme, const char *pszMood);
	bool SetTheme(SMusicTheme *pNewTheme, bool bOverride);
	bool SetMood(SMusicMood *pNewMood);
	SMusicMood* GetDefaultMood(SMusicTheme *pTheme);
	void PushPatternToMixList(SMusicPatternPlayInfo &PlayInfo);
	void EnterDefaultMood();
	void Silence();
	void AdjustMixStreamsRefCount(int nSamples);
	void MixStreams(void *pBuffer, int nLength);
	bool FadeStream(SMusicPatternPlayInfo &PlayInfo, signed short *pBuffer, int nSamples);
	void FadeOutAllSecondaryLayers(double fFadeTime);
	CMusicPatternInstance* GetPatternInstance(const char *pszPattern);
	bool UpdateCurrentPatternSet(SMusicMood *pMood, int nSamples, bool bAllowChange);
	bool ChoosePatternSet(SMusicMood *pMood);
	CMusicPatternInstance* ChooseBridge(SMusicTheme *pCurrTheme, SMusicTheme *pNewTheme);
	CMusicPatternInstance* ChoosePattern(SMusicMood *pMood, int nLayer=MUSICLAYER_MAIN);
	signed char StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength);
	//friend signed char __cdecl _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, INT_PTR nParam);
#ifndef CS_VERSION_372
#ifdef CS_VERSION_361
	friend signed char F_CALLBACKAPI _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, int nParam);
#else
	friend signed char F_CALLBACKAPI _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, INT_PTR nParam);
#endif
#else
  friend signed char F_CALLBACKAPI _StreamingCallback(CS_STREAM *pStream, void *pBuffer, int nLength, void * nParam);
#endif
  friend class CMusicLoadSink;
public:
	CMusicSystem(ISystem *pSystem);
	void Release();
	ISystem* GetSystem() { return m_pSystem; }
	int GetBytesPerSample() { return m_nBytesPerSample; }
	IMusicSystemSink* SetSink(IMusicSystemSink *pSink);
	bool SetData(struct SMusicData *pMusicData,bool bNoRelease=false);
	void ReleaseData();
	void Unload();
	void Pause(bool bPause);
	void EnableEventProcessing(bool bEnable) { m_bEnableEventProcessing=bEnable; }
	bool ResetThemeOverride();
	bool SetTheme(const char *pszTheme, bool bOverride=false);
	const char* GetTheme() { if (m_pCurrTheme) return m_pCurrTheme->sName.c_str(); return ""; }
	bool SetMood(const char *pszMood);
	bool SetDefaultMood(const char *pszMood);
	const char* GetMood() { if (m_pCurrMood) return m_pCurrMood->sName.c_str(); return ""; }
	IStringItVec* GetThemes();
	IStringItVec* GetMoods(const char *pszTheme);
	bool AddMusicMoodEvent(const char *pszMood, float fTimeout);
	void Update();
	SMusicSystemStatus* GetStatus();	// retrieve status of music-system... dont keep returning pointer !
	//! compute memory-consumption
	void GetMemoryUsage(class ICrySizer* pSizer);
	bool LoadMusicDataFromLUA(IScriptSystem* pScriptSystem, const char *pszFilename);
	bool StreamOGG();
	void LogMsg( const char *pszFormat, ... );

	//////////////////////////////////////////////////////////////////////////
	//! Load music data from XML.
	//! @param bAddAdata if true data from XML will be added to currently loaded music data.
	bool LoadFromXML( const char *sFilename,bool bAddData );

	//////////////////////////////////////////////////////////////////////////
	// Editing support.
	//////////////////////////////////////////////////////////////////////////
	virtual void UpdateTheme( SMusicTheme *pTheme );
	virtual void UpdateMood( SMusicMood *pMood );
	virtual void UpdatePattern( SPatternDef *pPattern );
	virtual void RenamePattern( const char *sOldName,const char *sNewName );
	virtual void PlayPattern( const char *sPattern,bool bStopPrevious );
	virtual void DeletePattern( const char *sPattern );

private:
	//////////////////////////////////////////////////////////////////////////
	// Loading.
	//////////////////////////////////////////////////////////////////////////
	void LoadPatternFromXML( XmlNodeRef node,SPatternDef *pPattern );
	void LoadMoodFromXML( XmlNodeRef &node,SMusicMood *pMood );
	void LoadThemeFromXML( XmlNodeRef &node,SMusicTheme *pTheme );
};

//////////////////////////////////////////////////////////////////////////
// String-vector-class to iterate data from other DLLs
//////////////////////////////////////////////////////////////////////////

class CStringItVec : public IStringItVec
{
public:
	CStringItVec(TStringVec &Vec)
	{
		m_nRefCount=0;
		m_Vec=Vec;
		MoveFirst();
	};
	bool IsEnd() { return (m_itVec==m_Vec.end()); };
	const char* Next() { return IsEnd() ? "" : (*m_itVec++).c_str(); };
	void MoveFirst() { m_itVec = m_Vec.begin(); };
	void AddRef() { m_nRefCount++; }
	void Release() { --m_nRefCount; if (m_nRefCount<=0) { delete this; } };
protected:
	int m_nRefCount;
	TStringVec m_Vec;
	TStringVecIt m_itVec;
};

//////////////////////////////////////////////////////////////////////////
// SmartCriticalSection-class to make things easier
//////////////////////////////////////////////////////////////////////////

class CSmartCriticalSection
{
private:
	CRITICAL_SECTION *m_pCS;
public:
	CSmartCriticalSection(CRITICAL_SECTION &CS) { m_pCS=&CS; EnterCriticalSection(m_pCS); }
	~CSmartCriticalSection() { LeaveCriticalSection(m_pCS); }
};