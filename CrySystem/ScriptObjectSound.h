// ScriptObjectSound.h: interface for the CScriptObjectSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTSOUND_H__A6B9BF56_1C4A_495C_A1B7_F0A052F6C05D__INCLUDED_)
#define AFX_SCRIPTOBJECTSOUND_H__A6B9BF56_1C4A_495C_A1B7_F0A052F6C05D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
struct IMusicSystem;
struct ISoundSystem;

/*! In this class are all sound-related script-functions implemented.

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectSound :
public _ScriptableEx<CScriptObjectSound>
{
public:
	//! constructor
	CScriptObjectSound();
	//! destructor
	virtual ~CScriptObjectSound();
	//!
	void Init(IScriptSystem *pScriptSystem, ISystem *pSystem);

	static void InitializeTemplate(IScriptSystem *pSS);

	int IsPlaying(IFunctionHandler *pH); //int (return int)
	int LoadSound(IFunctionHandler *pH); //char * (return int)
	int Load3DSound(IFunctionHandler *pH); //char * (return int)
	int LoadStreamSound(IFunctionHandler *pH); //char * (return int)	
	int PlaySound(IFunctionHandler *pH); //int (return void)	
	int SetSoundVolume(IFunctionHandler *pH); //int int (return void)
	int SetSoundLoop(IFunctionHandler *pH); //int int (return void)
	int SetSoundFrequency(IFunctionHandler *pH); //int int (return void)
	int SetSoundPitching(IFunctionHandler *pH); //int float (return void)
	int StopSound(IFunctionHandler *pH); // int (return void)
	int SetSoundPosition(IFunctionHandler *pH); //int vector(return void)
	int SetSoundSpeed(IFunctionHandler *pH); //int vector(return void)	
	int GetListener(IFunctionHandler *pH); // Not implemented yet
	int SetMinMaxDistance(IFunctionHandler *pH); // int float float float (return void)
	int SetLoopPoints(IFunctionHandler *pH); // int int int (return void)
	int AddSoundFlags(IFunctionHandler *pH); // int int (return void)
	int SetMasterVolumeScale(IFunctionHandler *pH); // int int (return void)
	int AddToScaleGroup(IFunctionHandler *pH); // int int (return void)
	int RemoveFromScaleGroup(IFunctionHandler *pH); // int int (return void)
	int SetGroupScale(IFunctionHandler *pH); // int int (return bool)
	int SetSoundProperties(IFunctionHandler *pH); // int int (return void)
	int	SetEaxEnvironment(IFunctionHandler *pH); // int (return void)
	int FXEnable(IFunctionHandler *pH);	// pSound, int nEffectNumber (return void)
	int SetFXSetParamEQ(IFunctionHandler *pH);	// pSound, float fCenter,float fBandwidth,float fGain (return void)
	int	SetDirectionalAttenuation(IFunctionHandler *pH);
	int	GetDirectionalAttenuationMaxScale(IFunctionHandler *pH);
	int	LoadMusic(IFunctionHandler *pH); // string (return bool)
	int	UnloadMusic(IFunctionHandler *pH);
	int	SetMusicTheme(IFunctionHandler *pH);
	int	ResetMusicThemeOverride(IFunctionHandler *pH);
	int	SetMusicMood(IFunctionHandler *pH);
	int	SetDefaultMusicMood(IFunctionHandler *pH);
	int	GetMusicThemes(IFunctionHandler *pH);
	int	GetMusicMoods(IFunctionHandler *pH);
	int AddMusicMoodEvent(IFunctionHandler *pH);
	int	IsInMusicTheme(IFunctionHandler *pH);
	int	IsInMusicMood(IFunctionHandler *pH);
	int GetMusicStatus(IFunctionHandler *pH);
	int SetSoundRatio(IFunctionHandler *pH);
	int PlaySoundFadeUnderwater(IFunctionHandler *pH);
	int GetSoundLength(IFunctionHandler * pH);

private: // -------------------------------------------------------

	ISystem *								m_pSystem;						//!<
	ISoundSystem *					m_pSoundSystem;				//!< might be 0 (e.g. dedicated server or Init() wasn't successful)
	IMusicSystem *					m_pMusicSystem;				//!< might be 0 (e.g. dedicated server or Init() wasn't successful)
};

#endif // !defined(AFX_SCRIPTOBJECTSOUND_H__A6B9BF56_1C4A_495C_A1B7_F0A052F6C05D__INCLUDED_)
