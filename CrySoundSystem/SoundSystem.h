//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2001
//
//  CrySound Source Code
//
//  File: SoundSystem.h
//  Description: Sound system interface.
//
//  History:
//	-June 06,2001:Implemented by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYSOUND_SOUNDSYSTEM_H
#define CRYSOUND_SOUNDSYSTEM_H

#include "SoundSystemCommon.h"
#include "SoundBuffer.h"

//////////////////////////////////////////////////////////////////////////////////////////////
class		CSound;
struct	ICVar;
class		CCamera;
struct	SoundBuffer;
struct	ITimer;
struct	IStreamEngine;

#define MAX_VIS_AREAS						256 // absolute maximum
#define MAX_ACTIVE_SOUNDSPOTS		16

#define SETSCALEBIT(bit) (1<<bit)

#include <Cry_Camera.h>
#define CSSPEAKERCONFIG_5POINT1			0x00000001
#define CSSPEAKERCONFIG_HEADPHONE		0x00000002
#define CSSPEAKERCONFIG_MONO				0x00000003
#define CSSPEAKERCONFIG_QUAD				0x00000004
#define CSSPEAKERCONFIG_STEREO			0x00000005
#define CSSPEAKERCONFIG_SURROUND		0x00000006

#define SFXFILTER_PRIORITY					CS_DSP_DEFAULTPRIORITY_SFXUNIT+1

struct SEAXProps
{
	CS_REVERB_PROPERTIES EAX;
	int nPreset;
};

enum ESoundActiveState
{
	eSoundState_None,
	eSoundState_Active,
	eSoundState_Inactive
};

//////////////////////////////////////////////////////////////////////////////////////////////
struct string_nocase_lt 
{
	bool operator()( const string &s1,const string &s2 ) const 
	{
		return stricmp(s1.c_str(),s2.c_str()) < 0;
	}
};
/*
struct SoundName
{
	SoundName(const char *_name,bool _b3d)
	{
		name=_name;
		b3d=_b3d;
	}
	SoundName(const SoundName& o)
	{
		name=o.name;
		b3d=o.b3d;
	}
	bool operator<(const SoundName& o)const
	{
		if(b3d!=o.b3d)
		{
			if (b3d)
				return true;
			return false;
		}
		return (stricmp(name.c_str(),o.name.c_str())<0);
	}
	string name;
	bool b3d;
};*/

typedef std::map<SSoundBufferProps, CSoundBuffer*>	SoundBufferPropsMap;
typedef SoundBufferPropsMap::iterator								SoundBufferPropsMapItor;

typedef std::vector<CSound*>	SoundVec;
typedef SoundVec::iterator		SoundVecItor;

typedef std::set<CSound*>	SoundList;
typedef SoundList::iterator		SoundListItor;

#ifdef WIN64
#undef PlaySound
#endif
//////////////////////////////////////////////////////////////////////////////////////////////
// Sound system interface
class CSoundSystem : public CSoundSystemCommon
{
protected:
	virtual ~CSoundSystem();
public:
	CSoundSystem(ISystem* pSystem, HWND hWnd);

	ISystem* GetSystem() const { return m_pISystem; };

	//! DSP unit callback for sfx-lowpass filter
	void* DSPUnit_SFXFilter_Callback(void *pOriginalBuffer, void *pNewBuffer, int nLength);

	//! retrieve sfx-filter dsp unit
	CS_DSPUNIT* GetDSPUnitFilter() { return m_pDSPUnitSFXFilter; }

	bool IsOK() { return m_bOK; }

	//!	Release the sound
	void Release();

	//!	Update the sound
	void Update();

	/*! Create a music-system. You should only create one music-system at a time.
	*/
	IMusicSystem* CreateMusicSystem();

	void SetSoundActiveState(CSound *pSound, ESoundActiveState State);

	//! Register new playing sound that should be auto stoped when it ends.
	void RegisterAutoStopSound( CSound *pSound );
	void UnregisterAutoStopSound( CSound *pSound );


	/*! Load a sound from disk
	@param szfile filename
	@param nFlags sound flags combination
	@return	sound interface
	*/
	ISound* LoadSound(const char *szFile, int nFlags);

	/*! Remove a sound from the sound system
	@param nSoundId sound id
	*/
	void RemoveSound(int nSoundID);

	/*! Add sound flags (OR)
	@param nSoundId sound id
	@param nFlags		additional flags
	*/		
	void AddSoundFlags(int nSoundID,int nFlags);

	/*! SetMasterVolume
	@param nVol volume (0-255)
	*/		
	void SetMasterVolume(unsigned char nVol)
	{
		GUARD_HEAP;
		CS_SetSFXMasterVolume(nVol);
	}
	/*! Set the volume scale for all sounds with FLAG_SOUND_SCALABLE
	@param fScale volume scale (default 1.0)
	*/		
	void SetMasterVolumeScale(float fScale, bool bForceRecalc=false);

	/*! Remove a sound reference from the sound system
	@param nSoundId sound id
	*/	
	void RemoveReference(CSound *);

	/*! Get a sound interface from the sound system
	@param nSoundId sound id
	*/	
	ISound* GetSound(int nSoundID);

	/*! Play a sound from the sound system
	@param nSoundId sound id
	*/	
	void PlaySound(int nSoundID);

	/*! Set the listener position
	@param vPos position
	@param vVel velocity
	@param vAngles angles
	*/	
	//void SetListener(const Vec3d &vPos, const Vec3d &vVel,const Vec3d& vAngles);
	void SetListener(const CCamera &cCam, const Vec3d &vVel);

	//! Sets minimal priority for sound to be played.
	int SetMinSoundPriority( int nPriority );
	int GetMinSoundPriority() { return m_nMinSoundPriority; };

	/*! to be called when something changes in the environment which could affect
	sound occlusion, for example a door closes etc.
	*/
	void	RecomputeSoundOcclusion(bool bRecomputeListener,bool bForceRecompute,bool bReset=false);

	void	FadeOutSound(CSound *pSound);

	//! Stop all sounds and music
	void	Silence();

	//! pause all sounds
	void	Pause(bool bPause,bool bResetVolume=false);

	//! Mute/unmute all sounds
	void	Mute(bool bMute);

	//! reset the sound system (between loading levels)
	void	Reset();

	//! Check for EAX support.
	bool IsEAX(int version);
	//! Set EAX listener environment.
	bool SetEaxListenerEnvironment(int nPreset, CS_REVERB_PROPERTIES *pProps=NULL, int nFlags=0);
	//! Gets current EAX listener environment.
	bool GetCurrentEaxEnvironment(int &nPreset, CS_REVERB_PROPERTIES &Props);

	bool SetGroupScale(int nGroup, float fScale);

	//! Will set speaker config
	void	SetSpeakerConfig();

	//! get memory usage info
	void	GetSoundMemoryUsageInfo(size_t &nCurrentMemory,size_t &nMaxMemory);

	//! get number of voices playing
	int	GetUsedVoices();

	//! get cpu-usuage
	float	GetCPUUsage();

	//! get music-volume
	float GetMusicVolume();

	//! sets parameters for directional attenuation (for directional microphone effect); set fConeInDegree to 0 to disable the effect
	void CalcDirectionalAttenuation(Vec3d &Pos, Vec3d &Dir, float fConeInRadians);

	//! returns the maximum sound-enhance-factor to use it in the binoculars as "graphical-equalizer"...
	float GetDirectionalAttenuationMaxScale() { return m_fDirAttMaxScale; }

	//! returns if directional attenuation is used
	bool UsingDirectionalAttenuation() { return (m_fDirAttCone!=0.0f); }

	//! remove a sound
	void RemoveBuffer(SSoundBufferProps &sn);

	//! compute memory-consumption
	void GetMemoryUsage(class ICrySizer* pSizer);

	//! get the current area the listener is in
	IVisArea	*GetListenerArea() { return(m_pVisArea); }
	Vec3d			GetListenerPos() {return(m_cCam.GetPos());}

	void BufferLoaded(CSoundBuffer *pSoundBuffer) { m_nBuffersLoaded++; }
	void BufferUnloaded(CSoundBuffer *pSoundBuffer) { m_nBuffersLoaded--; }

	//! Returns true if sound can be stopped.
	bool ProcessActiveSound( CSound *pSound );

	void LockResources();
	void UnlockResources();

	//! Return true.
	bool IsEnabled();

public:

	bool	m_bOK,m_bInside;
	bool	m_bValidPos;

	int m_nBuffersLoaded;

	//SoundMap			m_sounds;
	SoundBufferPropsMap	m_soundBuffers;
	SoundList	m_vecSounds; 
	SoundList	m_lstSoundSpotsInactive;
	SoundList	m_lstSoundSpotsActive;
	int				m_nInactiveSoundSpotsSize;
	SoundListItor m_itLastInactivePos;
	SoundList	m_lstFadeUnderwaterSounds;

	typedef std::set<_smart_ptr<CSound> > AutoStopSounds;
	AutoStopSounds m_autoStopSounds;
	//! This smart pointer list keep`s reference to stoped auto delete sounds, untill next update.
	std::vector<_smart_ptr<CSound> > m_stoppedSoundToBeDeleted;
	//int				m_nLastInactiveListPos;
	std::vector<_smart_ptr<CSoundBuffer> > m_lockedResources;

	// sfx-filter stuff //////////////////////////////////////////////////////
	CS_DSPUNIT *m_pDSPUnitSFXFilter; 
	float m_fDSPUnitSFXFilterCutoff;
	float m_fDSPUnitSFXFilterResonance;
	float m_fDSPUnitSFXFilterLowLVal;
	float m_fDSPUnitSFXFilterBandLVal;
	float m_fDSPUnitSFXFilterLowRVal;
	float m_fDSPUnitSFXFilterBandRVal;
	//////////////////////////////////////////////////////////////////////////

	float m_fSoundScale[MAX_SOUNDSCALE_GROUPS];

	ISystem	*m_pISystem;
	ITimer	*m_pTimer;
	ILog		*m_pILog;
	IStreamEngine *m_pStreamEngine;
	IVisArea	*m_pVisArea;
	IVisArea	*m_VisAreas[MAX_VIS_AREAS];
	char			m_szEmptyName[32];

	int m_nMuteRefCnt;

	float m_fSFXVolume;
	float m_fSFXResetVolume;
	float m_fMusicVolume;
	int m_nSampleRate;

	// this is used for the binocular-feature where sounds are heard according to their screen-projection
	Vec3d m_DirAttPos;
	Vec3d m_DirAttDir;
	float m_fDirAttCone;
	float m_fDirAttMaxScale;

	CCamera m_cCam;

	//listener infos
	Vec3d m_vPos,m_vForward,m_vTop;

	int		m_nSpeakerConfig;	
	int		m_nMinSoundPriority;
	bool	m_bPause,m_bResetVolume;

	int		m_nLastEax;
	CS_REVERB_PROPERTIES *m_pLastEAXProps;

	SEAXProps m_EAXIndoor;
	SEAXProps m_EAXOutdoor;

private:

	//! if sound is potentially hearable
	bool	IsSoundPH(CSound *pSound);
	void	AddToSoundSystem(CSound *pSound,int dwFlags);
	void	DeactivateSound( CSound *pSound );	
};

#endif // CRYSOUND_SOUNDSYSTEM_H
