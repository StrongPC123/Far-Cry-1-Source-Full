#include "StdAfx.h"
#include <ISystem.h>
#include <IConsole.h>
#include "soundsystemcommon.h"

CSoundSystemCommon::CSoundSystemCommon(ISystem* pSystem)
{
	m_pCVARDummySound= pSystem->GetIConsole()->CreateVariable("s_DummySound","0",VF_DUMPTODISK,
		"Toggles dummy(NULL) sound system.\n");	
	m_pCVARSoundEnable= pSystem->GetIConsole()->CreateVariable("s_SoundEnable","1",VF_DUMPTODISK,
		"Toggles sound on and off.\n"
		"Usage: s_SoundEnable [0/1]\n"
		"Default is 1 (on). Set to 0 to disable sound.");	
	m_pCVarMaxSoundDist= pSystem->GetIConsole()->CreateVariable("s_MaxSoundDist","500",VF_DUMPTODISK,
		"Sets the maximum sound distance, in metres. Default is 500 metres.\n"
		"Usage: s_MaxSoundDist 500");	
	m_pCVarDopplerEnable = pSystem->GetIConsole()->CreateVariable("s_DopplerEnable","0",VF_DUMPTODISK,
		"Toggles Doppler effect on and off.\n"
		"Usage: s_DopplerEnable [0/1]\n"
		"Default is 0 (off).");	
	m_pCVarDopplerValue = pSystem->GetIConsole()->CreateVariable("s_DopplerValue","1.0",VF_DUMPTODISK,
		"Sets the strength of the Doppler effect.\n"
		"Usage: s_DopplerValue 1.0"
		"Default is 1.0. This multiplier affects the sound velocity.");	
	m_pCVarSFXVolume = pSystem->GetIConsole()->CreateVariable("s_SFXVolume","1",VF_DUMPTODISK,
		"Sets the percentile volume of the sound effects.\n"
		"Usage: s_SFXVolume 0.5\n"
		"Default is 1, which is full volume.");	
	m_pCVarMusicVolume = pSystem->GetIConsole()->CreateVariable("s_MusicVolume","0.5",VF_DUMPTODISK,
		"Sets the music volume from 0 to 1.\n"
		"Usage: s_MusicVolume 7\n"
		"Default is 1, which is full volume.");	
	m_pCVarSampleRate = pSystem->GetIConsole()->CreateVariable("s_SampleRate","44100",VF_DUMPTODISK,
		"Sets the output sample rate.\n"
		"Usage: s_SampleRate 44100\n"
		"Default is 44100. Sets the rate, in samples per second,\n"
		"at which the output of the sound system is played.");
	m_pCVarSpeakerConfig = pSystem->GetIConsole()->CreateVariable("s_SpeakerConfig","5",VF_DUMPTODISK,
		"Sets up the preferred speaker configuration.\n"
		"Usage: s_SpeakerConfig #\n"
		"where # is a number between 1 and 6 representing\n"
		"	1: 5.1\n"
		"	2: Headphones\n"
		"	3: Mono\n"
		"	4: Quad\n"
		"	5: Stereo\n"
		"	6: Surround\n"
		"Default is 5 (Stereo).");
	m_pCVarEnableSoundFX = pSystem->GetIConsole()->CreateVariable("s_EnableSoundFX","1",VF_DUMPTODISK,
		"Toggles sound effects on and off.\n"
		"Usage: s_EnableSoundFX [0/1]\n"
		"Default is 1 (on).");
	m_pCVarDebugSound = pSystem->GetIConsole()->CreateVariable("s_DebugSound","0",VF_CHEAT,
		"Toggles sound debugging mode.\n"
		"Usage: s_DebugSound [0/1]\n" 
		"Default is 0 (off).");
	m_pCVarSoundInfo = pSystem->GetIConsole()->CreateVariable("s_SoundInfo","0",VF_CHEAT,
		"Toggles onscreen sound statistics.\n"
		"Usage: s_SoundInfo [0/1]"
		"Default is 0 (off). Set to 1 to display sound information.");
	m_pCVarInactiveSoundIterationTimeout = pSystem->GetIConsole()->CreateVariable("s_InactiveSoundIterationTimeout","1",VF_DUMPTODISK,
		"This variable is for internal use only.");
	m_pCVarMinHWChannels = pSystem->GetIConsole()->CreateVariable("s_MinHWChannels","16",VF_DUMPTODISK,
		"Sets the minimum number of sound hardware channels used./n"
		"Usage: s_MinHWChannels 16\n"
		"Default value is 16. This is the number of hardware channels\n"
		"used to play back game sound. If the hardware does not have\n"
		"the required channels, the system will use software mixing,\n"
		"resulting in reduced performance (no eax)."); 
	// 28 = hw mixing
	m_pCVarMaxHWChannels= pSystem->GetIConsole()->CreateVariable("s_MaxHWChannels","28",VF_DUMPTODISK,
		"Sets the maximum number of sound hardware channels used./n"
		"0=software mixing.");
	m_pCVarVisAreaProp=pSystem->GetIConsole()->CreateVariable("s_VisAreasPropagation","10",VF_DUMPTODISK,
		"Sets the vis area propagation number.\n"
		"Usage: s_VisAreasPropagation 5\n"
		"Default is 10. This number defines how far sound will\n"
		"propagate, in vis areas from the player's position. A\n"
		"value of 1 means the sound is only heard in the current\n"
		"vis area, while 2 means sound is heard in adjacent areas."); 
	m_pCVarMaxSoundSpots=pSystem->GetIConsole()->CreateVariable("s_MaxActiveSoundSpots","100",VF_DUMPTODISK,
		"Sets the maximum number of active soundspots.\n");
	m_pCVarMinRepeatSoundTimeout = pSystem->GetIConsole()->CreateVariable("s_MinRepeatSoundTimeout","200",VF_DUMPTODISK,
		"Timeout in ms,play of same sounds cannot be repeated more often then that.\n");		
	m_pCVarCompatibleMode = pSystem->GetIConsole()->CreateVariable("s_CompatibleMode","0",VF_DUMPTODISK,
		"Toggles sound system compatible mode ON/OFF. Use only in case of playback issues.\n");		
	m_pCVarCapsCheck= pSystem->GetIConsole()->CreateVariable("s_CapsCheck","0",VF_DUMPTODISK,
		"Checks for sound CAPS on startup and write them into the log.\n");	
}

CSoundSystemCommon::~CSoundSystemCommon()
{
	m_pCVARDummySound->Release();
	m_pCVARSoundEnable->Release();
	m_pCVarMaxSoundDist->Release();	
	m_pCVarDopplerEnable->Release();
	m_pCVarDopplerValue->Release();
	m_pCVarSFXVolume->Release();
	m_pCVarMusicVolume->Release();
	m_pCVarSampleRate->Release();
	m_pCVarSpeakerConfig->Release();
	m_pCVarEnableSoundFX->Release();
	m_pCVarDebugSound->Release();
	m_pCVarSoundInfo->Release();
	m_pCVarInactiveSoundIterationTimeout->Release();
	m_pCVarMinHWChannels->Release();
	m_pCVarMaxHWChannels->Release();
	m_pCVarVisAreaProp->Release();
	m_pCVarMaxSoundSpots->Release();
	m_pCVarMinRepeatSoundTimeout->Release();
	m_pCVarCompatibleMode->Release();
	m_pCVarCapsCheck->Release();
}

bool CSoundSystemCommon::DebuggingSound()
{
	return (m_pCVarDebugSound!=NULL) && (m_pCVarDebugSound->GetIVal()!=0);
}