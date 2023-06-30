#pragma once

struct ISystem;
struct ICVar;

class CSoundSystemCommon : public ISoundSystem
{
public:

	CSoundSystemCommon(ISystem* pSystem);
	virtual ~CSoundSystemCommon();
	bool DebuggingSound();

	//enable playback of sounds
	ICVar *m_pCVARSoundEnable;

	ICVar *m_pCVARDummySound;

	//max sound distance
	ICVar *m_pCVarMaxSoundDist;

	ICVar	*m_pCVarDopplerEnable;
	ICVar	*m_pCVarDopplerValue;

	ICVar	*m_pCVarSFXVolume;
	ICVar	*m_pCVarMusicVolume;
	ICVar	*m_pCVarSampleRate;
	ICVar *m_pCVarSpeakerConfig;
	ICVar	*m_pCVarEnableSoundFX;
	ICVar	*m_pCVarDebugSound;
	ICVar *m_pCVarSoundInfo;
	ICVar *m_pCVarInactiveSoundIterationTimeout;
	ICVar *m_pCVarMinHWChannels;
	ICVar *m_pCVarMaxHWChannels;
	ICVar *m_pCVarVisAreaProp;
	ICVar *m_pCVarMaxSoundSpots;
	ICVar *m_pCVarMinRepeatSoundTimeout;
	ICVar *m_pCVarCompatibleMode;
	ICVar *m_pCVarCapsCheck;
};
