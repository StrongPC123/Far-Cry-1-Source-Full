//////////////////////////////////////////////////////////////////////
//
//  CrySound Source Code
//
//  File: Sound.h
//  Description: ISound interface implementation.
//
//  History:
//	-June 06,2001:Implemented by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYSOUND_SOUND_H
#define CRYSOUND_SOUND_H

//////////////////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include "Cry_Math.h"
#include "SoundSystem.h"
#include <IStreamEngine.h>

struct CS_STREAM;
struct CS_SAMPLE;
struct IVisArea;
struct ITimer;

#define SOUND_PRELOAD_DISTANCE	15	// start preloading this amount of meters before the sound is potentially hearable (max_radius)

//////////////////////////////////////////////////////////////////////////////////////////////
// A sound...
class CSound : public ISound
{
protected:
	virtual ~CSound();
public:
	CSound(class CSoundSystem *pSSys,const char *szFile);
		
	//// ISound //////////////////////////////////////////////////////
	bool IsPlayingOnChannel();	// returns true if a voice is used to play the sound and have channel allocated.
	bool IsPlaying();	// returns true if a voice is used to play the sound
	bool IsPlayingVirtual(); // returns true if the sound is looping and not stopped. it might be too far (clipped) so it will not use a physical voice nor is it audioable
	void Play(float fVolumeScale=1.0f, bool bForceActiveState=true, bool bSetRatio=true);
	void PlayFadeUnderwater(float fVolumeScale=1.0f, bool bForceActiveState=true, bool bSetRatio=true);
	void Stop();
	//! Frees fmod channel, also stop hearable sound playback
	void FreeChannel();
	//void SetName(const char *szName);

	//! Get name of sound file.
	const char *GetName();

	//! Get uniq id of sound.
	const int GetId();

	//! Set looping mode of sound.
	void SetLoopMode(bool bLoop);	

	//! retrieves the currently played sample-pos, in milliseconds or bytes
	unsigned int GetCurrentSamplePos(bool bMilliSeconds);
	//! set the currently played sample-pos in bytes or milliseconds
	void SetCurrentSamplePos(unsigned int nPos,bool bMilliSeconds);	

	//! sets automatic pitching amount (0-1000)
	void SetPitching(float fPitching);

	void SetBaseFrequency(int nFreq);

	//! Return frequency of sound.
	int	 GetBaseFrequency(int nFreq);

	void SetFrequency(int nFreq);	
	//! Set Minimal/Maximal distances for sound.
	//! Sound is not attenuated below minimal distance and not heared outside of max distance.	
	void SetMinMaxDistance(float fMinDist, float fMaxDist);
	void SetAttrib(int nVolume, float fRatio, int nPan=127, int nFreq=1000, bool bSetRatio=true);
	void SetAttrib(const Vec3d &pos, const Vec3d &speed);
	void SetRatio(float fRatio);

	//! Update the position (called when directional-attenuation-parameters change)
	void UpdatePosition();

	//! Set sound source position.
	void SetPosition(const Vec3d &pos);

	//! Get sound source position.
	//@return false if it is not a 3d sound	
	const bool GetPosition(Vec3d &vPos);	

	void	SetLooping(bool bLooping) {}
	int		GetFrequency() 
	{ 
		if (m_pSound)
			return (m_pSound->GetBaseFreq());
		return (0);
	}

	//! Set sound pitch.
	//! 1000 is default pitch.	
	void	SetPitch(int nValue); 	

	//! Set sound pan
	void	SetPan(int nPan);

	//! Define sound code.
	//! Angles are in degrees, in range 0-360.	
	void	SetConeAngles(float fInnerAngle,float fOuterAngle);

	//! Set sound volume.
	//! Range: 0-100	
	void	SetVolume(int);

	//! Get sound volume.
	int		GetVolume() { return(m_nVolume); }

	//! Set sound source velocity.
	void	SetVelocity(const Vec3d &vVel);

	//! Get sound source velocity.
	Vec3_tpl<float>	GetVelocity( void ) { return (m_speed); }

	//! Set orientation of sound.
	//! Only relevant when cone angles are specified.	
	void	SetDirection(const Vec3d &vDir);
	Vec3_tpl<float>	GetDirection() {return (m_orient);}	

	void	SetLoopPoints(const int iLoopStart, const int iLoopEnd);

	//! compute fmod flags from crysound-flags
	int GetFModFlags();
	//! load the sound only when it is played
	bool	Preload();

	bool IsRelative() const
	{ 
		if (m_nFlags & FLAG_SOUND_RELATIVE)
			return true;
		else
			return false;
	}

	// Sets certain sound properties  
	void SetSoundProperties(float fFadingValue);	

	// Add/remove sounds.
	int	AddRef() { return ++m_refCount; };
	int	Release();

	//! enable fx effects for this sound
	//! _ust be called after each play
	void	FXEnable(int nEffectNumber);

	void	FXSetParamEQ(float fCenter,float fBandwidth,float fGain);	

	void	AddToScaleGroup(int nGroup) { m_nSoundScaleGroups|=(1<<nGroup); }
	void	RemoveFromScaleGroup(int nGroup) { m_nSoundScaleGroups&=~(1<<nGroup); }
	void	SetScaleGroup(unsigned int nGroupBits) { m_nSoundScaleGroups=nGroupBits; }

	//! set the maximum distance / the sound will be stopped if the
	//! distance from the listener and this sound is bigger than this max distance
//	void	SetMaxSoundDistance(float fMaxSoundDistance);

	//! returns the size of the stream in ms
	int GetLengthMs();

	//! returns the size of the stream 
	int GetLength();

	//! set sound priority (0-255)
	void	SetSoundPriority(unsigned char nSoundPriority);

	void	ChangeVolume(int nVolume);

	int		CalcSoundVolume(int nSoundValue,float fVolumeScale);

	bool	FadeIn();
	bool	FadeOut();

	void AddEventListener( ISoundEventListener *pListener );
	void RemoveEventListener( ISoundEventListener *pListener );
	//! Fires event for all listeners to this sound.
	void OnEvent( ESoundCallbackEvent event );

	void OnBufferLoaded();
	void OnBufferLoadFailed();

	void	GetMemoryUsage(class ICrySizer* pSizer);

	//! Check if sound now loading (not yet played).
	bool IsLoading();
	bool IsLoaded();
	bool IsUsingChannel();

	bool IsPlayLengthExpired( float fCurrTime  );
	
public:

	int		m_nChannel;
	int		m_nFxChannel;	
	//int		m_nLastId;
	//int		m_nFadeType;
	int		m_nStartOffset;
	
	CSoundBufferPtr m_pSound;

	float m_fLastPlayTime;
	float m_fChannelPlayTime;
	float m_fSoundLengthSec;

	unsigned int m_nSoundScaleGroups;

	ESoundActiveState m_State;

	int		m_nMaxVolume;
	int		m_nVolume;
	int		m_nPlayingVolume;
	int		m_nPan;
	float	m_fPitching;	// relative random pitching of sound (to avoid flanging if multiple sounds are played simultaneously)
	int		m_nCurrPitch;
	//int		m_nBaseFreq;
	int		m_nRelativeFreq;
	bool	m_bLoop;	
	bool	m_bPlaying;
	float m_fRatio;
	
	Vec3_tpl<float>	m_position;
	Vec3_tpl<float>	m_orient;
	Vec3_tpl<float>	m_speed;
	int m_refCount;
	int   m_nFlags;
	Vec3_tpl<float>	m_RealPos;	// this is the position the sound is currently at (may differ from m_position, because for example the directional attenuation will virtually "move" the sound)

	//string m_strName;
	
	float	m_fMaxRadius2; //used to skip playing of sounds if using sound's sphere
	float	m_fMinRadius2; //used to skip playing of sounds if using sound's sphere
	float	m_fDiffRadius2; //used to calculate volume ratio when inside the sound's sphere
	float m_fPreloadRadius2;	// used to start preloading
	float	m_fMinDist,m_fMaxDist;	
///	float m_fMaxSoundDistance2; //keep it squared

	class CSoundSystem* m_pSSys;
	ITimer *m_pTimer;

	//short		m_nSectorId,m_nBuildingId;
	IVisArea	*m_pArea;
	float			m_fFadingValue,m_fCurrentFade;

	bool			m_bPlayAfterLoad;
	float			m_fPostLoadRatio;
	bool			m_bPostLoadForceActiveState;
	bool			m_bPostLoadSetRatio;
	bool			m_bAutoStop;
	bool			m_bAlreadyLoaded;

	unsigned char m_nSoundPriority;

	static int m_PlayingChannels;

	typedef std::set<ISoundEventListener*> Listeners;
	Listeners m_listeners;
};

#endif // CRYSOUND_SOUND_H
