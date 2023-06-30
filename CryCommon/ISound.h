//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2001
//
//  CrySound Source Code
//
//  File: ISound.h
//  Description: Sound interface.
// 
//  History:
//  - August 28, 2001: Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYSOUND_ISOUND_H
#define CRYSOUND_ISOUND_H

//forward declarations
//////////////////////////////////////////////////////////////////////

#include "Cry_Math.h"
#include <vector>
#include <map>
#include "StlUtils.h"
#include "TString.h"

class		CCamera;
struct	IMusicSystem;
class		ICrySizer;
struct	IVisArea;

//////////////////////////////////////////////////////////////////////
#define MAX_SFX			1024

//////////////////////////////////////////////////////////////////////
#define FLAG_SOUND_LOOP									1<<0 
#define FLAG_SOUND_2D										1<<1
#define FLAG_SOUND_3D										1<<2 
#define FLAG_SOUND_STEREO								1<<3 
#define FLAG_SOUND_16BITS								1<<4 
#define FLAG_SOUND_STREAM								1<<5 
#define FLAG_SOUND_RELATIVE							1<<6   // sound position moves relative to player
#define FLAG_SOUND_RADIUS								1<<7 	// sound has a radius, custom attenuation calculation
#define FLAG_SOUND_DOPPLER							1<<8 	// use doppler effect for this sound	
#define FLAG_SOUND_NO_SW_ATTENUATION		1<<9 	// doesn't use SW attenuation for this sound
#define FLAG_SOUND_MUSIC								1<<10 	// pure music sound, to use to set pure music volume
#define FLAG_SOUND_OUTDOOR							1<<11 	// play the sound only if the listener is in outdoor
#define FLAG_SOUND_INDOOR	 							1<<12	// play the sound only if the listener is in indoor
#define FLAG_SOUND_UNSCALABLE						1<<13 	// for all sounds with this flag the volume can be scaled separately respect to the master volume
#define FLAG_SOUND_OCCLUSION	 					1<<14 	// the sound uses sound occlusion
//#define FLAG_SOUND_FAILED				32768 // the loading of this sound has failed - do not try to load again every frame
#define FLAG_SOUND_LOAD_SYNCHRONOUSLY		1<<15  // the loading of this sound will be synchronous (asynchronously by default).
#define FLAG_SOUND_FADE_OUT_UNDERWATER  1<<16 

#define FLAG_SOUND_ACTIVELIST	 	(FLAG_SOUND_RADIUS | FLAG_SOUND_OCCLUSION | FLAG_SOUND_INDOOR | FLAG_SOUND_OUTDOOR)

#define SOUNDBUFFER_FLAG_MASK	(FLAG_SOUND_LOOP | FLAG_SOUND_2D | FLAG_SOUND_3D | FLAG_SOUND_STEREO | FLAG_SOUND_16BITS | FLAG_SOUND_STREAM)	// flags affecting the sound-buffer, not its instance

#define MAX_SOUNDSCALE_GROUPS		8

#define SOUNDSCALE_MASTER				0
#define SOUNDSCALE_SCALEABLE		1
#define SOUNDSCALE_DEAFNESS			2
#define SOUNDSCALE_UNDERWATER		3
#define SOUNDSCALE_MISSIONHINT	4

struct ISound;

//These values are used with CS_FX_Enable to enable DirectX 8 FX for a channel.
//////////////////////////////////////////////////////////////////////////////////////////////
enum SOUND_FX_MODES
{
    S_FX_CHORUS,
    S_FX_COMPRESSOR,
    S_FX_DISTORTION,
    S_FX_ECHO,
    S_FX_FLANGER,
    S_FX_GARGLE,
    S_FX_I3DL2REVERB,
    S_FX_PARAMEQ,
    S_FX_WAVES_REVERB
};

//eax modes
//////////////////////////////////////////////////////////////////////////////////////////////
enum {
	EAX_PRESET_OFF=0,              
	EAX_PRESET_GENERIC,          
	EAX_PRESET_PADDEDCELL,       
	EAX_PRESET_ROOM, 	           
	EAX_PRESET_BATHROOM, 	       
	EAX_PRESET_LIVINGROOM,       
	EAX_PRESET_STONEROOM,        
	EAX_PRESET_AUDITORIUM,       
	EAX_PRESET_CONCERTHALL,      
	EAX_PRESET_CAVE,             
	EAX_PRESET_ARENA,            
	EAX_PRESET_HANGAR,           
	EAX_PRESET_CARPETTEDHALLWAY, 
	EAX_PRESET_HALLWAY,          
	EAX_PRESET_STONECORRIDOR,    
	EAX_PRESET_ALLEY, 	       
	EAX_PRESET_FOREST, 	       
	EAX_PRESET_CITY,             
	EAX_PRESET_MOUNTAINS,        
	EAX_PRESET_QUARRY,           
	EAX_PRESET_PLAIN,            
	EAX_PRESET_PARKINGLOT,       
	EAX_PRESET_SEWERPIPE,        
	EAX_PRESET_UNDERWATER       
};

//! Sound events sent to callback that can registered to every sound.
enum ESoundCallbackEvent
{
	SOUND_EVENT_ON_LOADED,			//!< Fired when sound is loaded.
	SOUND_EVENT_ON_LOAD_FAILED,	//!< Fired if sound loading is failed.
	SOUND_EVENT_ON_PLAY,				//!< Fired when sound is begin playing on channel.
	SOUND_EVENT_ON_STOP					//!< Fired when sound stops being playing on channel and frees channel.
};

//////////////////////////////////////////////////////////////////////////
//! Listener interface for the sound.
//////////////////////////////////////////////////////////////////////////
struct ISoundEventListener
{
	//! Callback event.
	virtual void OnSoundEvent( ESoundCallbackEvent event,ISound *pSound ) = 0;
};

// Marco's NOTE: this is a redefine of the EAX preset OFF, since it seems
// that audigy cards are having problems when the default EAX off preset
#define MY_CS_PRESET_OFF  {0,	1.0f,	0.00f, -10000, -10000, -10000,   0.1f,  0.1f, 0.1f,  -10000, 0.0f, { 0.0f,0.0f,0.0f }, -10000, 0.0f, { 0.0f,0.0f,0.0f }, 0.0750f, 0.00f, 0.04f, 0.000f, 0.0f, 1000.0f, 20.0f, 0.0f,   0.0f,   0.0f, 0 }

//////////////////////////////////////////////////////////////////////////////////////////////
//crysound definitions
#ifdef WIN64
#include <CrySound64.h>
#else
#include <CrySound.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Sound system interface
struct ISoundSystem
{
	virtual void Release() = 0;
	virtual void Update() = 0;

	/*! Create a music-system. You should only create one music-system at a time.
	*/
	virtual IMusicSystem* CreateMusicSystem() = 0;

	/*! Load a sound from disk
	@param szfile filename
	@param nFlags sound flags combination
	@return	sound interface
	*/	
	virtual struct ISound* LoadSound(const char *szFile, int nFlags) = 0;

	/*! SetMasterVolume
	@param nVol volume (0-255)
	*/		
	virtual void SetMasterVolume(unsigned char nVol) = 0;

	/*! Set the volume scale for all sounds with FLAG_SOUND_SCALABLE
	@param fScale volume scale (default 1.0)
	*/			
	virtual void SetMasterVolumeScale(float fScale, bool bForceRecalc=false) = 0;

	/*! Get a sound interface from the sound system
	@param nSoundId sound id
	*/		
	virtual struct ISound* GetSound(int nSoundID) = 0;

	/*! Play a sound from the sound system
	@param nSoundId sound id
	*/		
	virtual void PlaySound(int nSoundID) = 0;

	/*! Set the listener position
	@param cCam camera position
	@param vVel velocity	
	*/			
	virtual void SetListener(const CCamera &cCam, const Vec3 &vVel)=0;

	/*! to be called when something changes in the environment which could affect
	sound occlusion, for example a door closes etc.
	@param	bRecomputeListener recomputes the listener vis area
	@param	bForceRecompute forces to recompute the vis area connections even if
					the listener didn't move (useful for moving objects that can occlude)
	*/
	virtual void	RecomputeSoundOcclusion(bool bRecomputeListener,bool bForceRecompute,bool bReset=false)=0;
	
	//! Check for EAX support.
	virtual bool IsEAX( int version ) = 0;
	//! Set EAX listener environment; one of the predefined presets
	//! listened above or a custom environmental reverb set
	virtual bool SetEaxListenerEnvironment(int nPreset, CS_REVERB_PROPERTIES *pProps=NULL, int nFlags=0) = 0;

	//! Gets current EAX listener environment or one of the predefined presets
	//! used to save into the savegame
	virtual bool GetCurrentEaxEnvironment(int &nPreset, CS_REVERB_PROPERTIES &Props)=0;

	//! Set the scaling factor for a specific scale group (0-31)
	virtual bool SetGroupScale(int nGroup, float fScale) = 0;

	//! Stop all sounds and music
	virtual void	Silence()=0;

	//! pause all sounds
	virtual void	Pause(bool bPause,bool bResetVolume=false)=0; 

	//! Mute/unmute all sounds
	virtual void	Mute(bool bMute)=0;

	//! get memory usage info
	virtual void	GetSoundMemoryUsageInfo(size_t &nCurrentMemory,size_t &nMaxMemory)=0;

	//! get number of voices playing
	virtual int	GetUsedVoices()=0;

	//! get cpu-usuage
	virtual float	GetCPUUsage()=0;

	//! get music-volume
	virtual float GetMusicVolume()=0;

	//! sets parameters for directional attenuation (for directional microphone effect); set fConeInDegree to 0 to disable the effect
	virtual void CalcDirectionalAttenuation(Vec3 &Pos, Vec3 &Dir, float fConeInRadians) = 0;

	//! returns the maximum sound-enhance-factor to use it in the binoculars as "graphical-equalizer"...
	virtual float GetDirectionalAttenuationMaxScale() = 0;

	//! returns if directional attenuation is used
	virtual bool UsingDirectionalAttenuation() = 0;

	virtual void GetMemoryUsage(class ICrySizer* pSizer) = 0;

	//! get the current area the listener is in
	virtual IVisArea	*GetListenerArea()=0;

	//! get listener position
	virtual	Vec3	GetListenerPos()=0;

	//! returns true if sound is being debugged
	virtual bool DebuggingSound()=0;	

	//! Set minimal priority for sounds to be played.
	//! Sound`s with priority less then that will not be played.
	//! @return previous minimal priority.
	virtual int SetMinSoundPriority( int nPriority ) = 0;

	//! Lock all sound buffer resources to prevent them from unloading (when restoring checkpoint).
	virtual void LockResources() = 0;
	//! Unlock all sound buffer resources to prevent them from unloading.
	virtual void UnlockResources() = 0;
};

//////////////////////////////////////////////////////////////////////////
// String Iterator
struct IStringItVec
{
	virtual bool IsEnd() = 0;
	virtual const char* Next() = 0;
	virtual void MoveFirst() = 0;
	virtual void AddRef() = 0;
	virtual void Release() = 0;
};

//////////////////////////////
// A sound...
struct ISound
{
	// Register listener to the sound.
	virtual void AddEventListener( ISoundEventListener *pListener ) = 0;
	virtual void RemoveEventListener( ISoundEventListener *pListener ) = 0;

	virtual bool IsPlaying() = 0;
	virtual bool IsPlayingVirtual() = 0;
	//! Return true if sound is now in the process of asynchronous loading of sound buffer.
	virtual bool IsLoading() = 0;
	//! Return true if sound have already loaded sound buffer.
	virtual bool IsLoaded() = 0;

	virtual void Play(float fVolumeScale=1.0f, bool bForceActiveState=true, bool bSetRatio=true) = 0;
	virtual void PlayFadeUnderwater(float fVolumeScale=1.0f, bool bForceActiveState=true, bool bSetRatio=true) = 0;
	virtual void Stop() = 0;

	//! Get name of sound file.
	virtual const char*	GetName() = 0;
	//! Get uniq id of sound.
	virtual const int		GetId() = 0;

	//! Set looping mode of sound.
	virtual void SetLoopMode(bool bLoop) = 0;

	virtual bool Preload() = 0;
 
	//! retrieves the currently played sample-pos, in milliseconds or bytes
	virtual unsigned int GetCurrentSamplePos(bool bMilliSeconds=false)=0;
	//! set the currently played sample-pos in bytes or milliseconds
	virtual void SetCurrentSamplePos(unsigned int nPos,bool bMilliSeconds)=0;

	//! sets automatic pitching amount (0-1000)
	virtual void SetPitching(float fPitching) = 0;

	//! sets the volume ratio
	virtual void SetRatio(float fRatio)=0;

	//! Return frequency of sound.
	virtual int	 GetFrequency() = 0;
	
	//! Set sound pitch.
	//! 1000 is default pitch.
	virtual void SetPitch(int nPitch) = 0;

	//! Set panning values
	virtual void	SetPan(int nPan)=0;

	//! set the maximum distance / the sound will be stopped if the
	//! distance from the listener and this sound is bigger than this max distance
//	virtual void SetMaxSoundDistance(float fMaxSoundDistance)=0;

	//! Set Minimal/Maximal distances for sound.
	//! Sound is not attenuated below minimal distance and not heared outside of max distance.
	virtual void SetMinMaxDistance(float fMinDist, float fMaxDist) = 0;

	//! Define sound cone.
	//! Angles are in degrees, in range 0-360.
	virtual void SetConeAngles(float fInnerAngle,float fOuterAngle) = 0;

	//! Add sound to specific sound-scale-group (0-31)
	virtual void AddToScaleGroup(int nGroup) = 0;
	//! Remove sound from specific sound-scale-group (0-31)
	virtual void RemoveFromScaleGroup(int nGroup) = 0;
	//! Set sound-scale-groups by bitfield.
	virtual void SetScaleGroup(unsigned int nGroupBits) = 0;

	//! Set sound volume.
	//! Range: 0-100
	virtual	void	SetVolume( int nVolume ) = 0;
	//! Get sound volume.
	virtual	int		GetVolume() = 0;

	//! Set sound source position.
	//IVO
	virtual void	SetPosition(const Vec3 &pos) = 0;

	//! Get sound source position.
	virtual const bool GetPosition(Vec3 &vPos) = 0;
	
	//! Set sound source velocity.
	virtual void	SetVelocity(const Vec3 &vel) = 0;
	//! Get sound source velocity.
	virtual Vec3	GetVelocity( void ) = 0;

	//! Set orientation of sound.
	//! Only relevant when cone angles are specified.
	virtual void	SetDirection( const Vec3 &dir ) = 0;
	virtual Vec3	GetDirection() = 0;

	virtual void SetLoopPoints(const int iLoopStart, const int iLoopEnd) = 0;
	virtual bool IsRelative() const = 0;

	// Add/remove sounds.
	virtual int	AddRef() = 0;
	virtual int	Release() = 0;

	/* Sets certain sound properties  
	//@param	fFadingValue	the value that should be used for fading / sound occlusion
	// more to come
	*/
	virtual void SetSoundProperties(float fFadingValue)=0;

	//virtual void	AddFlags(int nFlags) = 0;

	//! enable fx effects for this sound
	//! must be called after each play
	virtual	void	FXEnable(int nEffectNumber)=0;

	virtual	void	FXSetParamEQ(float fCenter,float fBandwidth,float fGain)=0;

	//! returns the size of the stream in ms
	virtual int GetLengthMs()=0;

	//! returns the size of the stream in bytes
	virtual int GetLength()=0;

	//! set sound priority (0-255)
	virtual void	SetSoundPriority(unsigned char nSoundPriority)=0;	
};

//////////////////////////////////////////////////////////////////////////
// MusicSystem

//////////////////////////////////////////////////////////////////////////
// Structures to pass as data-entry for musicsystem
//////////////////////////////////////////////////////////////////////////

struct SPatternDef;
struct SMusicPatternSet;
struct SMusicMood;
struct SMusicTheme;

// Helper integer-vector
typedef std::vector<int>	TIntVec;
typedef TIntVec::iterator	TIntVecIt;

typedef std::vector<SPatternDef*>							TPatternDefVec;
typedef TPatternDefVec::iterator							TPatternDefVecIt;

typedef std::map<CryBasicString,string,stl::less_stricmp<CryBasicString> > TThemeBridgeMap;
typedef TThemeBridgeMap::iterator							TThemeBridgeMapIt;

typedef std::map<CryBasicString,SMusicMood*,stl::less_stricmp<CryBasicString> > TMoodMap;
typedef TMoodMap::iterator										TMoodMapIt;

typedef std::vector<SMusicPatternSet*>				TPatternSetVec;
typedef TPatternSetVec::iterator							TPatternSetVecIt;

typedef std::map<CryBasicString,SMusicTheme*,stl::less_stricmp<CryBasicString> >	TThemeMap;
typedef TThemeMap::iterator										TThemeMapIt;

// Pattern-definition
struct SPatternDef
{
	CryBasicString sName;
	CryBasicString sFilename;
	TIntVec vecFadePoints;
	int nLayeringVolume;
	float fProbability;

	SPatternDef()
	{
		nLayeringVolume = 255;
		fProbability = 0;
	}
};

// PatternSet-Structure used by moods
struct SMusicPatternSet
{
	float fMinTimeout;
	float fMaxTimeout;
	float fTotalMainPatternProbability;	// added probabilities of all containing patterns
	TPatternDefVec vecMainPatterns;
	int nMaxSimultaneousRhythmicPatterns;
	float fRhythmicLayerProbability;
	float fTotalRhythmicPatternProbability;	// added probabilities of all containing patterns
	TPatternDefVec vecRhythmicPatterns;
	int nMaxSimultaneousIncidentalPatterns;
	float fIncidentalLayerProbability;
	float fTotalIncidentalPatternProbability;	// added probabilities of all containing patterns
	TPatternDefVec vecIncidentalPatterns;

	SMusicPatternSet()
	{
		fMinTimeout = 0;
		fMaxTimeout = 0;
		fRhythmicLayerProbability = 0;
		fTotalMainPatternProbability = 0;
		fTotalRhythmicPatternProbability = 0;
		fIncidentalLayerProbability = 0;
		fTotalIncidentalPatternProbability = 0;
		nMaxSimultaneousRhythmicPatterns = 1;
		nMaxSimultaneousIncidentalPatterns = 1;
	}
};

// Mood-Structure
struct SMusicMood
{
	CryBasicString sName;
	int nPriority;
	float fFadeOutTime;
	TPatternSetVec vecPatternSets;
	bool bPlaySingle;
	// internal
	SMusicPatternSet *pCurrPatternSet;
	float fCurrPatternSetTime;
	float fCurrPatternSetTimeout;

	SMusicMood()
	{
		nPriority = 0;
		fFadeOutTime = 0;
		bPlaySingle = false;
		pCurrPatternSet = NULL;
		fCurrPatternSetTime = 0;
		fCurrPatternSetTimeout = 0;
	}
};

// Theme-Structure
struct SMusicTheme
{
	CryBasicString sName;
	TMoodMap mapMoods;
	TThemeBridgeMap mapBridges;
	// default mood
	CryBasicString sDefaultMood;
	float fDefaultMoodTimeout;

	SMusicTheme()
	{
		fDefaultMoodTimeout = 0;
	}
};

// Data-struct (which needs to be passed to SetData())
struct SMusicData
{
	TPatternDefVec vecPatternDef;
	TThemeMap mapThemes;
};

// Different layers
#define MUSICLAYER_MAIN							0
#define MUSICLAYER_RHYTHMIC					1
#define MUSICLAYER_INCIDENTAL				2

#define DEFAULT_CROSSFADE_TIME			3.0

//////////////////////////////////////////////////////////////////////////
// Status struct
//////////////////////////////////////////////////////////////////////////

struct SPlayingPatternsStatus
{
	CryBasicString sName;
	int nLayer;
	int nVolume;
};

typedef std::vector<SPlayingPatternsStatus>		TPatternStatusVec;
typedef TPatternStatusVec::iterator						TPatternStatusVecIt;

struct SMusicSystemStatus
{
	bool bPlaying;
	CryBasicString sTheme;
	CryBasicString sMood;
	TPatternStatusVec m_vecPlayingPatterns;
};

//////////////////////////////////////////////////////////////////////////
// Main music-interface
//////////////////////////////////////////////////////////////////////////

struct IMusicSystem
{
	virtual void Release() = 0;
	virtual struct ISystem* GetSystem() = 0;
	virtual int GetBytesPerSample() = 0;
	virtual struct IMusicSystemSink* SetSink(struct IMusicSystemSink *pSink) = 0;
	virtual bool SetData(struct SMusicData *pMusicData,bool bNoRelease=false) = 0;
	virtual void Unload() = 0;
	virtual void Pause(bool bPause) = 0;
	virtual void EnableEventProcessing(bool bEnable) = 0;
	virtual bool ResetThemeOverride() = 0;
	virtual bool SetTheme(const char *pszTheme, bool bOverride=false) = 0;
	virtual const char* GetTheme() = 0;
	virtual bool SetMood(const char *pszMood) = 0;
	virtual bool SetDefaultMood(const char *pszMood) = 0;
	virtual const char* GetMood() = 0;
	virtual IStringItVec* GetThemes() = 0;
	virtual IStringItVec* GetMoods(const char *pszTheme) = 0;
	virtual bool AddMusicMoodEvent(const char *pszMood, float fTimeout) = 0;
	virtual void Update() = 0;
	virtual SMusicSystemStatus* GetStatus() = 0;		// retrieve status of music-system... dont keep returning pointer !
	virtual void GetMemoryUsage(class ICrySizer* pSizer) = 0;
	virtual bool LoadMusicDataFromLUA(struct IScriptSystem* pScriptSystem, const char *pszFilename) = 0;
	virtual bool StreamOGG() = 0;
	virtual void LogMsg( const char *pszFormat, ... ) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Load music data from XML.
	//! @param bAddAdata if true data from XML will be added to currently loaded music data.
	virtual bool LoadFromXML( const char *sFilename,bool bAddData ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Editing support.
	//////////////////////////////////////////////////////////////////////////
	virtual void UpdateTheme( SMusicTheme *pTheme ) = 0;
	virtual void UpdateMood( SMusicMood *pMood ) = 0;
	virtual void UpdatePattern( SPatternDef *pPattern ) = 0;
	virtual void RenamePattern( const char *sOldName,const char *sNewName ) = 0;
	virtual void PlayPattern( const char *sPattern,bool bStopPrevious ) = 0;
	virtual void DeletePattern( const char *sPattern ) = 0;
	virtual void Silence() = 0;
};

//////////////////////////////////////////////////////////////////////////
// Sink to release data (if allocated in a different DLL
//////////////////////////////////////////////////////////////////////////

struct IMusicSystemSink
{
	virtual void ReleaseData(struct SMusicData *pData) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////
typedef ISoundSystem* (*PFNCREATESOUNDSYSTEM)(struct ISystem*, void*);

#ifdef WIN32
extern "C" 
#ifdef CRYSOUNDSYSTEM_EXPORTS
	#define CRYSOUND_API __declspec(dllexport)
#else
	#define CRYSOUND_API __declspec(dllimport)
#endif
#else //WIN32
	#define CRYSOUND_API 
#endif //WIN32

extern "C"
{
	CRYSOUND_API ISoundSystem* CreateSoundSystem(struct ISystem*, void *pInitData);
}

#endif // CRYSOUND_ISOUND_H
