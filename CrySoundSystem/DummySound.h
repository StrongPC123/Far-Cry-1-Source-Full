
//////////////////////////////////////////////////////////////////////
//
//  CrySound Source Code
//
//  File: DummySound.h
//  Description: DummySound interface implementation.
//
//  History:
//	-June 06,2001:Implemented by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef _DUMMY_SOUND_H_
#define _DUMMY_SOUND_H_

#include "SoundSystemCommon.h"
#include "DummyMusic.h"
 
/////////////////////////////////////////////////////////////////
class CDummySound : public ISound
{
public:
	CDummySound()
	{		
		m_refCount = 0;
	}

	~CDummySound(){}
	
	//// ISound //////////////////////////////////////////////////////
	bool	IsPlaying(){ return false; }
	bool	IsPlayingVirtual(){ return false; }
	bool	IsLoaded() { return true; };
	bool	IsLoading() { return false; };
	void	Play(float fVolumeScale=1.0f, bool bForceActiveState=true, bool bSetRatio=true){}
	void	PlayFadeUnderwater(float fVolumeScale=1.0f, bool bForceActiveState=true, bool bSetRatio=true) {};
	void	Stop(){}
	void	SetName(const char *szName){}
	const char *GetName(){return "dummy sound";}
	const int GetId(){return 0; }
	void	SetLoopMode(bool bLoop){}
	unsigned int GetCurrentSamplePos(){return 0;}
	void	SetPitching(float fPitching){}
	void	SetBaseFrequency(int nFreq){}
	int		GetBaseFrequency(int nFreq){return 44100;}
	void	SetFrequency(int nFreq){}
	void	SetMinMaxDistance(float fMinDist, float fMaxDist){}
	void	SetAttrib(int nVolume, int nPan=127, int nFreq=1000, bool bSetRatio=true){}
	void	SetAttrib(const Vec3d &pos, const Vec3d &speed){}
	void	SetRatio(float fRatio){}
	void	SetPosition(const Vec3d &pos){}
	const bool GetPosition(Vec3d &vPos){return (false);}
	void	SetSpeed(const Vec3d &speed){}
	const Vec3d& GetSpeed(){return m_vDummy;}
	void	SetLooping(bool bLooping) {}
	void	AddToScaleGroup(int nGroup) {}
	void	RemoveFromScaleGroup(int nGroup) {}
	void	SetScaleGroup(unsigned int nGroupBits) {}
	int		GetFrequency() { return (0);}
	void	SetPitch(int nValue) {}	
	void	SetPan(int nPan) {}
	void	SetMaxSoundDistance(float fMaxSoundDistance) {}
	void	SetMinMaxDistance(float,float,float) {}
	void	SetConeAngles(float,float) {}
	void	SetVolume(int) {}
	int		GetVolume() { return(0); }
	void	SetVelocity(const Vec3d &vVel) {}
	Vec3d	GetVelocity() {return (Vec3d(0,0,0));}
	void	SetDirection(const Vec3d &vDir) {}
	Vec3d	GetDirection() {return (Vec3d(0,0,0));}	
	void    SetLoopPoints(const int iLoopStart, const int iLoopEnd) { };
	bool    IsRelative() const { return false; };
	bool	RegisterInIndoor() { return(false); };
	void	SetSoundPriority(unsigned char nSoundPriority) {} ;
	bool  Preload() { return true; }
	// Sets certain sound properties  
	void SetSoundProperties(float fFadingValue) {} ;	

	void AddFlags(int nFlags) {}
	int		AddRef() { return ++m_refCount; };
	int		Release()
	{
		int ref = --m_refCount;
		return ref;
	};

	//! enable fx effects for this sound
	//! must be called after each play
	void	FXEnable(int nEffectNumber) {}
	void	FXSetParamEQ(float fCenter,float fBandwidth,float fGain) {}

	//! retrieves the currently played sample-pos, in milliseconds or bytes
	unsigned int GetCurrentSamplePos(bool bMilliSeconds=false) { return (0);}
	//! set the currently played sample-pos in bytes or milliseconds
	void SetCurrentSamplePos(unsigned int nPos,bool bMilliSeconds) {}

	//! returns the size of the stream in ms
	int GetLengthMs() { return(0); }

	//! returns the size of the stream in bytes
	int GetLength() { return(0); }
	void AddEventListener( ISoundEventListener *pListener ) {};
	void RemoveEventListener( ISoundEventListener *pListener ) {};

private:
	Vec3d m_vDummy;	
	int		m_refCount;	
};

#ifdef WIN64
#undef PlaySound
#endif

/////////////////////////////////////////////////////////////////
class CDummySoundSystem : public CSoundSystemCommon
{
public:
	CDummySoundSystem(ISystem* pSystem, HWND hWnd) : CSoundSystemCommon(pSystem) {}
	~CDummySoundSystem()
  {

  }

	bool IsOK() { return true; }

	//// ISoundSystem ///////////////////////////////////////////////
	void		Release(){}
	void		Update(){}
	IMusicSystem* CreateMusicSystem() { return new CDummyMusicSystem(NULL); }
	ISound* LoadSound(const char *szFile, int nFlags) { return &m_sndDummy; }
	void		RemoveSound(int nSoundID){}
	ISound* GetSound(int nSoundID){ return &m_sndDummy;}
	void SetMasterVolume(unsigned char nVol) { }
	void SetMasterVolumeScale(float fScale, bool bForceRecalc=false){}
	void AddSoundFlags(int nSoundID,int nFlags){}

	void		PlaySound(int nSoundID){}
	//void		SetListener(const Vec3d &vPos,const Vec3d &vVel,const Vec3d& vAngles) {}
	void		SetListener(const CCamera &cCam, const Vec3d &vVel) {}
	Vec3d		GetListenerPos() {return(Vec3d(0,0,0));}

	bool	PlayMusic(const char *szFileName) { return (false); }
	void	StopMusic() {}
	void	Silence() {}	
	void	Pause(bool bPause,bool bResetVolume=false) {}
	void	Mute(bool bMute) {}
	
	//! Check for EAX support.
	bool IsEAX(int version) { return(false); }
	//! Set EAX listener environment.
	bool SetEaxListenerEnvironment(int nPreset,CS_REVERB_PROPERTIES *pProps=NULL, int nFlags=0) { return(false); }
		//! Gets current EAX listener environment.
	bool GetCurrentEaxEnvironment(int &nPreset, CS_REVERB_PROPERTIES &Props) { nPreset=0;return (true);}
	bool SetGroupScale(int nGroup, float fScale) { return true; }
	void RecomputeSoundOcclusion(bool bRecomputeListener,bool bForceRecompute,bool bReset) {}

	//! get memory usage info
	void	GetSoundMemoryUsageInfo(size_t &nCurrentMemory,size_t &nMaxMemory)
	{
		nCurrentMemory=nMaxMemory=0;
	}

	int	GetUsedVoices() { return 0; }
	float	GetCPUUsage() { return 0.0f; }
	float GetMusicVolume() { return 0.0f; }
	void CalcDirectionalAttenuation(Vec3d &Pos, Vec3d &Dir, float fConeInRadians) {}
	float GetDirectionalAttenuationMaxScale() { return 0.0f; }
	bool UsingDirectionalAttenuation() { return false; }
	void GetMemoryUsage(class ICrySizer* pSizer) {}
	//! get the current area the listener is in
	IVisArea	*GetListenerArea() { return(NULL); }
	int SetMinSoundPriority( int nPriority ) { return 0; };
	
	void LockResources() {};
	void UnlockResources() {};

private:
	CDummySound m_sndDummy;
};


#endif //_DUMMY_SOUND_H_