// ScriptObjectSound.cpp: implementation of the CScriptObjectSound class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptObjectSound.h"
#include <ScriptObjectVector.h>
#include <ISystem.h>
#include <IConsole.h>
#include <ILog.h>
#include <ISound.h>
#ifdef WIN64
#include <CrySound64.h>
#else
#include <CrySound.h>
#endif
 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


_DECLARE_SCRIPTABLEEX(CScriptObjectSound)

CScriptObjectSound::CScriptObjectSound()
{
	m_pSoundSystem=0;
	m_pMusicSystem=0;
	m_pSystem=0;
}

CScriptObjectSound::~CScriptObjectSound()
{
}

/*! Initializes the script-object and makes it available for the scripts.
		@param pScriptSystem Pointer to the ScriptSystem-interface
		@param pGame Pointer to the Game
		@param pSystem Pointer to the System-interface
		@see IScriptSystem
		@see CXGame
		@see ISystem
*/
void CScriptObjectSound::Init(IScriptSystem *pScriptSystem, ISystem *pSystem)
{
	m_pSystem=pSystem;
	m_pSoundSystem=m_pSystem->GetISoundSystem();
	m_pMusicSystem=m_pSystem->GetIMusicSystem();
	InitGlobal(pScriptSystem,"Sound",this);

	m_pScriptSystem->SetGlobalValue("SOUND_RELATIVE",FLAG_SOUND_RELATIVE);
	m_pScriptSystem->SetGlobalValue("SOUND_RADIUS",FLAG_SOUND_RADIUS);
	m_pScriptSystem->SetGlobalValue("SOUND_DOPPLER",FLAG_SOUND_DOPPLER);
	m_pScriptSystem->SetGlobalValue("SOUND_NO_SW_ATTENUATION",FLAG_SOUND_NO_SW_ATTENUATION);
	m_pScriptSystem->SetGlobalValue("SOUND_MUSIC",FLAG_SOUND_MUSIC);
	m_pScriptSystem->SetGlobalValue("SOUND_OUTDOOR",FLAG_SOUND_OUTDOOR);
	m_pScriptSystem->SetGlobalValue("SOUND_INDOOR",FLAG_SOUND_INDOOR);
	m_pScriptSystem->SetGlobalValue("SOUND_UNSCALABLE",FLAG_SOUND_UNSCALABLE);
	m_pScriptSystem->SetGlobalValue("SOUND_LOOP",FLAG_SOUND_LOOP);
	m_pScriptSystem->SetGlobalValue("SOUND_OCCLUSION",FLAG_SOUND_OCCLUSION);
	m_pScriptSystem->SetGlobalValue("SOUND_STREAM",FLAG_SOUND_STREAM);
	m_pScriptSystem->SetGlobalValue("SOUND_STEREO",FLAG_SOUND_STEREO);
	m_pScriptSystem->SetGlobalValue("SOUND_FADE_OUT_UNDERWATER",FLAG_SOUND_FADE_OUT_UNDERWATER);
	m_pScriptSystem->SetGlobalValue("SOUND_LOAD_SYNCHRONOUSLY",FLAG_SOUND_LOAD_SYNCHRONOUSLY);	
	
	m_pScriptSystem->SetGlobalValue("SOUNDSCALE_MASTER", SOUNDSCALE_MASTER);
	m_pScriptSystem->SetGlobalValue("SOUNDSCALE_SCALEABLE", SOUNDSCALE_SCALEABLE);
	m_pScriptSystem->SetGlobalValue("SOUNDSCALE_DEAFNESS", SOUNDSCALE_DEAFNESS);
	m_pScriptSystem->SetGlobalValue("SOUNDSCALE_UNDERWATER", SOUNDSCALE_UNDERWATER);
	m_pScriptSystem->SetGlobalValue("SOUNDSCALE_MISSIONHINT", SOUNDSCALE_MISSIONHINT);

	m_pScriptSystem->SetGlobalValue("SOUND_VOLUMESCALEMISSIONHINT", 0.45f);
}

void CScriptObjectSound::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectSound>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectSound,IsPlaying);
	REG_FUNC(CScriptObjectSound,LoadSound);
	REG_FUNC(CScriptObjectSound,Load3DSound);
	REG_FUNC(CScriptObjectSound,LoadStreamSound);
	REG_FUNC(CScriptObjectSound,PlaySound);	
	REG_FUNC(CScriptObjectSound,PlaySoundFadeUnderwater);	
	REG_FUNC(CScriptObjectSound,SetSoundVolume);
	REG_FUNC(CScriptObjectSound,SetSoundLoop);
	REG_FUNC(CScriptObjectSound,SetSoundFrequency);
	REG_FUNC(CScriptObjectSound,SetSoundPitching);
	REG_FUNC(CScriptObjectSound,StopSound);
	REG_FUNC(CScriptObjectSound,SetSoundPosition);
	REG_FUNC(CScriptObjectSound,SetSoundSpeed);	
	REG_FUNC(CScriptObjectSound,GetListener);
	REG_FUNC(CScriptObjectSound,SetMinMaxDistance);
	REG_FUNC(CScriptObjectSound,SetLoopPoints);	
	REG_FUNC(CScriptObjectSound,SetMasterVolumeScale);
	REG_FUNC(CScriptObjectSound,AddToScaleGroup);
	REG_FUNC(CScriptObjectSound,RemoveFromScaleGroup);
	REG_FUNC(CScriptObjectSound,SetGroupScale);
	REG_FUNC(CScriptObjectSound,SetSoundProperties);
	REG_FUNC(CScriptObjectSound,SetEaxEnvironment);
	REG_FUNC(CScriptObjectSound,FXEnable);
	REG_FUNC(CScriptObjectSound,SetFXSetParamEQ);
	REG_FUNC(CScriptObjectSound,SetDirectionalAttenuation);
	REG_FUNC(CScriptObjectSound,GetDirectionalAttenuationMaxScale);
	REG_FUNC(CScriptObjectSound,LoadMusic);
	REG_FUNC(CScriptObjectSound,UnloadMusic);
	REG_FUNC(CScriptObjectSound,ResetMusicThemeOverride);
	REG_FUNC(CScriptObjectSound,SetMusicTheme);
	REG_FUNC(CScriptObjectSound,SetMusicMood);
	REG_FUNC(CScriptObjectSound,SetDefaultMusicMood);
	REG_FUNC(CScriptObjectSound,GetMusicThemes);
	REG_FUNC(CScriptObjectSound,GetMusicMoods);
	REG_FUNC(CScriptObjectSound,AddMusicMoodEvent);
	REG_FUNC(CScriptObjectSound,IsInMusicTheme);
	REG_FUNC(CScriptObjectSound,IsInMusicMood);
	REG_FUNC(CScriptObjectSound,GetSoundLength);
	REG_FUNC(CScriptObjectSound,GetMusicStatus);
	REG_FUNC(CScriptObjectSound,SetSoundRatio);	
}

/*! Determine if a sound is still playing
	@param nID ID of the sound
	@return nil or not nil
*/
int CScriptObjectSound::IsPlaying(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nCookie=0;
	bool bRes=false;
	ISound *pSound=NULL;

	pH->GetParamUDVal(1,(USER_DATA&)pSound,nCookie);		//AMD Port
 	if (pSound && (nCookie==USER_DATA_SOUND))
	{
 		if (pSound->IsPlaying())
 			bRes=true;
 		else
 			bRes=false;
 	}
  
	return pH->EndFunction(bRes);
}

/*! Load a 2D sound
	@param sSound filename of the sound
	@return nil or sound ID in case of success
*/
int CScriptObjectSound::LoadSound(IFunctionHandler *pH)
{
	float fVolume=255.0f;

	if (pH->GetParamCount()<1 || pH->GetParamCount()>3)
	{
		m_pScriptSystem->RaiseError("Sound.LoadSound wrong number of arguments");
		return pH->EndFunctionNull();
	};
	
	const char *sSound;
	ISound *pSound;
	int nFlags=0;
	int pcount=pH->GetParamCount();
//	int iLocalized;
	pH->GetParam(1,sSound);

	if (pcount>1) 
		pH->GetParam(2, nFlags);

	if (pcount>2)
		pH->GetParam(3, fVolume);

  if (m_pSoundSystem)
  {
  	pSound=m_pSoundSystem->LoadSound(sSound,nFlags);
    if(pSound)
    {
			pSound->SetVolume((int)(fVolume));
						
			pSound->AddRef();
			USER_DATA ud=m_pScriptSystem->CreateUserData((INT_PTR)pSound,USER_DATA_SOUND);		//AMD Port
			return pH->EndFunction(ud);
  	}
		else{
			m_pScriptSystem->RaiseError("Sound.LoadSound error loading %s",sSound);
		}
  }
	return pH->EndFunctionNull();
}



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Load a 3D sound
	@param sSound filename of the sound
	@param nFlags flags :) [optional]
	@param nVolume [optional]
	@param nMinDistance [optional]
	@param nMaxDistance [optional]
	@return nil or sound ID in case of success
*/
int CScriptObjectSound::Load3DSound(IFunctionHandler *pH)
{
	int nParamCount=pH->GetParamCount();
	if(nParamCount<1)
	{
		m_pScriptSystem->RaiseError("Sound.Load3DSound wrong number of arguments");
		return pH->EndFunctionNull();
	};
	
	const char *sSound=0;
	ISound *pSound;
	int nFlags=0;

	if (!pH->GetParam(1, sSound))
	{
		m_pScriptSystem->RaiseError("Load3dSound: First parameter not a string");
		return pH->EndFunction(-1);
	}

	float fMin=1,fClipDistance=500,fMax=100000,fVolume=128;
	int	nPriority=0;

	if (nParamCount>1)
	{
		pH->GetParam(2, nFlags);
	}
	if (nParamCount>2)
	{
		pH->GetParam(3,fVolume);
	}

	if (nParamCount>3)
	{
		pH->GetParam(4,fMin);
	}

	if (nParamCount>4)
	{
		pH->GetParam(5,fClipDistance);
		if (fClipDistance>1000)
			fClipDistance=1000;
		//fMax=fClipDistance;
	}

	if (pH->GetParamCount()>5) 
	{
		pH->GetParam(6, nPriority);
	}

	if (m_pSoundSystem)
	{			
		pSound=m_pSoundSystem->LoadSound(sSound, FLAG_SOUND_3D | nFlags);

		if (pSound)
		{
			pSound->SetVolume((int)fVolume);
			pSound->SetMinMaxDistance(fMin,fClipDistance/2.0f);
			//				pSound->SetMaxSoundDistance(fClipDistance/2); // :) 				
			if (pH->GetParamCount()>6) 
			{
				unsigned int nGroups;
				pH->GetParam(7, nGroups);
				pSound->SetScaleGroup(nGroups);
			}
			pSound->AddRef();
			USER_DATA ud=m_pScriptSystem->CreateUserData((INT_PTR)pSound,USER_DATA_SOUND);	//AMD Port
			pSound->SetSoundPriority(nPriority);
			return pH->EndFunction(ud);
		}
		else
		{
			//m_pScriptSystem->RaiseError("Sound.Load3DSound error loading %s",sSound);
			m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,
				sSound,"Sound %s Failed to Load",sSound );
		}
	}

	return pH->EndFunctionNull();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Load a 3D sound
@param sSound filename of the sound
@param nFlags flags :) [optional]
@param nVolume [optional]
@param nMinDistance [optional]
@param nMaxDistance [optional]
@return nil or sound ID in case of success
*/
/*
int CScriptObjectSound::Load3DSoundLocalized(IFunctionHandler *pH)
{
	int nParamCount=pH->GetParamCount();
	if(nParamCount<1)
	{
		m_pScriptSystem->RaiseError("Sound.Load3DSound wrong number of arguments");
		return pH->EndFunctionNull();
	};

	const char *sSound=0;
	ISound *pSound;
	int nFlags=0;

	if (!pH->GetParam(1, sSound))
	{
		m_pScriptSystem->RaiseError("Load3dSound: First parameter not a string");
		return pH->EndFunction(-1);
	}

	float fMin=1,fClipDistance=500,fMax=100000,fVolume=128;
	int	nPriority=0;

	if (nParamCount>1)
	{
		pH->GetParam(2, nFlags);
	}
	if (nParamCount>2)
	{
		pH->GetParam(3,fVolume);
	}

	if (nParamCount>3)
	{
		pH->GetParam(4,fMin);
	}

	if (nParamCount>4)
	{
		pH->GetParam(5,fClipDistance);
		if (fClipDistance>1000)
			fClipDistance=1000;
		//fMax=fClipDistance;
	}

	if (pH->GetParamCount()>5) 
	{
		pH->GetParam(6, nPriority);
	}

	string szPath;

	ICVar *g_language = m_pSystem->GetIConsole()->GetCVar("g_language");
	assert(g_language);

	szPath = "LANGUAGES/";
	szPath += g_language->GetString();
	szPath += "/";
	szPath += sSound;


	if (m_pSoundSystem)
	{			
		pSound=m_pSoundSystem->LoadSound(szPath.c_str(), FLAG_SOUND_3D | nFlags);

		if (pSound)
		{
			pSound->SetVolume((int)fVolume);
			pSound->SetMinMaxDistance(fMin,fClipDistance/2.0f);
			//				pSound->SetMaxSoundDistance(fClipDistance/2); // :) 				
			if (pH->GetParamCount()>6) 
			{
				unsigned int nGroups;
				pH->GetParam(7, nGroups);
				pSound->SetScaleGroup(nGroups);
			}
			pSound->AddRef();
			USER_DATA ud=m_pScriptSystem->CreateUserData((INT_PTR)pSound,USER_DATA_SOUND);	//AMD Port
			pSound->SetSoundPriority(nPriority);
			return pH->EndFunction(ud);
		}
		else
		{
			//m_pScriptSystem->RaiseError("Sound.Load3DSound error loading %s",sSound);
			m_pSystem->Warning( VALIDATOR_MODULE_SYSTEM,VALIDATOR_WARNING,VALIDATOR_FLAG_SOUND,
				sSound,"Sound %s Failed to Load",szPath.c_str() );
		}
	}

	return pH->EndFunctionNull();
}
*/

/*!	set the Environment model(EAX)
sets one of the eax presets specified in isound.h
@param nPreset one of the presets
*/
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::SetEaxEnvironment(IFunctionHandler *pH)
{
	if (pH->GetParamCount()<1)
	{
		CHECK_PARAMETERS(1);
	}
	_SmartScriptObject pObj(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3 vVec;
	int nEaxEnvironment=0;

	/*
	    nEnvironment=0,		--sets all listener properties (win32/ps2 only) 
	    fEnvSize=1.0,		--environment size in meters (win32 only) 
	    fEnvDiffusion=0.0,		--environment diffusion (win32/xbox) 
	    nRoom=-10000,		--room effect level (at mid frequencies) (win32/xbox/ps2) 
	    nRoomHF=-10000,		--relative room effect level at high frequencies (win32/xbox) 
	    nRoomLF=-10000,		--relative room effect level at low frequencies (win32 only) 
	    fDecayTime=0.1,		--reverberation decay time at mid frequencies (win32/xbox) 
	    fDecayHFRatio=0.1,		--high-frequency to mid-frequency decay time ratio (win32/xbox) 
	    fDecayLFRatio=0.1,		--low-frequency to mid-frequency decay time ratio (win32 only) 
	    nReflections=-10000,	--early reflections level relative to room effect (win32/xbox) 
	    fReflectionsDelay=0.0,	--initial reflection delay time (win32/xbox) 
	    fReflectionsPan={x=0,y=0,z=0},	--early reflections panning vector (win32 only) 
	    nReverb=-10000,		--late reverberation level relative to room effect (win32/xbox) 
	    fReverbDelay=0.0,		--late reverberation delay time relative to initial reflection (win32/xbox) 
	    fReverbPan={x=0,y=0,z=0},		--late reverberation panning vector (win32 only) 
	    fEchoTime=0.075,		--echo time (win32 only) 
	    fEchoDepth=0.0,		--echo depth (win32 only) 
	    fModulationTime=0.04,	--modulation time (win32 only) 
	    fModulationDepth=0.0,	--modulation depth (win32 only) 
	    fAirAbsorptionHF=-100,	--change in level per meter at high frequencies (win32 only) 
	    fHFReference=1000.0,	--reference high frequency (hz) (win32/xbox) 
	    fLFReference=20.0,		--reference low frequency (hz) (win32 only) 
	    fRoomRolloffFactor=0.0,	--like CS_3D_Listener_SetRolloffFactor but for room effect (win32/xbox) 
	    fDiffusion=0.0,		--Value that controls the echo density in the late reverberation decay. (xbox only) 
	    fDensity=0.0,		--Value that controls the modal density in the late reverberation decay (xbox only) 
	    nFlags=0,			--CS_REVERB_FLAGS - modifies the behavior of above properties (win32 only) 
	*/

	CS_REVERB_PROPERTIES pProps;

	int nTemp; //cannot use unsigned int as parameter to getvaluechain function

	INT_PTR nFlags=0;
	if (pH->GetParamCount()>=2)
		pH->GetParam(2, nFlags);

	//check if there is a table containing all EAX parameters
	if (pH->GetParam(1,pObj))
	{
		pObj->BeginSetGetChain();
		pObj->GetValueChain("nEnvironment",nTemp);pProps.Environment=(unsigned int)nTemp;
		pObj->GetValueChain("fEnvSize",pProps.EnvSize);
		pObj->GetValueChain("fEnvDiffusion",pProps.EnvDiffusion);
		pObj->GetValueChain("nRoom",pProps.Room);
		pObj->GetValueChain("nRoomHF",pProps.RoomHF);
		pObj->GetValueChain("nRoomLF",pProps.RoomLF);
		pObj->GetValueChain("fDecayTime",pProps.DecayTime);
		pObj->GetValueChain("fDecayHFRatio",pProps.DecayHFRatio);
		pObj->GetValueChain("fDecayLFRatio",pProps.DecayLFRatio);
		pObj->GetValueChain("nReflections",pProps.Reflections);
		pObj->GetValueChain("fReflectionsDelay",pProps.ReflectionsDelay);		
		pObj->GetValueChain("fReflectionsPan",oVec);vVec=oVec.Get();
		pProps.ReflectionsPan[0]=vVec.x;pProps.ReflectionsPan[1]=vVec.y;pProps.ReflectionsPan[2]=vVec.z;
		pObj->GetValueChain("nReverb",pProps.Reverb);
		pObj->GetValueChain("fReverbDelay",pProps.ReverbDelay);
		pObj->GetValueChain("fReverbPan",oVec);vVec=oVec.Get();
		pProps.ReverbPan[0]=vVec.x;pProps.ReverbPan[1]=vVec.y;pProps.ReverbPan[2]=vVec.z;		
		pObj->GetValueChain("fEchoTime",pProps.EchoTime);
		pObj->GetValueChain("fEchoDepth",pProps.EchoDepth);
		pObj->GetValueChain("fModulationTime",pProps.ModulationTime);
		pObj->GetValueChain("fModulationDepth",pProps.ModulationDepth);
		pObj->GetValueChain("fAirAbsorptionHF",pProps.AirAbsorptionHF);
		pObj->GetValueChain("fHFReference",pProps.HFReference);
		pObj->GetValueChain("fLFReference",pProps.LFReference);
		pObj->GetValueChain("fRoomRolloffFactor",pProps.RoomRolloffFactor);
		pObj->GetValueChain("fDiffusion",pProps.Diffusion);
		pObj->GetValueChain("fDensity",pProps.Density);
		pObj->GetValueChain("nFlags",nTemp);pProps.Flags=(unsigned int)nTemp;
		pObj->EndSetGetChain();

		if (m_pSoundSystem)		
			m_pSoundSystem->SetEaxListenerEnvironment(0,&pProps,nFlags);		
	}
	else
	{
		//use one of the pre-defined presets
		if (pH->GetParam(1, nEaxEnvironment))
		{
			if (m_pSoundSystem)		
				m_pSoundSystem->SetEaxListenerEnvironment(nEaxEnvironment,NULL,nFlags);		
		}
	}

	return pH->EndFunction();
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Load a streaming sound
	@param sSound filename of the sound
	@return nil or sound ID in case of success
*/
int CScriptObjectSound::LoadStreamSound(IFunctionHandler *pH)
{	
	if (pH->GetParamCount()<1 || pH->GetParamCount()>2)
	{  
		m_pScriptSystem->RaiseError("System.LoadStreamSound wrong number of arguments"); 
		return pH->EndFunctionNull(); 
	};
 
/*
	ICVar *pMusic=m_pSystem->GetIConsole()->GetCVar("g_MusicEnable");
	if (pMusic && (pMusic->GetIVal()==0))
		return (pH->EndFunctionNull());
*/
	const char *sSound;
//	int nID;
	ISound *pSound;
	pH->GetParam(1,sSound);

	int nFlags=0;
	if (pH->GetParamCount()>1) 
		pH->GetParam(2, nFlags);

  if (m_pSoundSystem)
  {
	  pSound=m_pSoundSystem->LoadSound(sSound,FLAG_SOUND_STREAM | nFlags);
 	  if (pSound)
		{
			pSound->AddRef();
			USER_DATA ud=m_pScriptSystem->CreateUserData((INT_PTR)pSound,USER_DATA_SOUND);	//AMD Port
			return pH->EndFunction(ud);
  	}
  }else
	{
			m_pScriptSystem->RaiseError("Sound.LoadStreamSound error loading %s",sSound);
	}
	return pH->EndFunctionNull();

}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Load a streaming sound
@param sSound filename of the sound
@return nil or sound ID in case of success
*/
/*
int CScriptObjectSound::LoadStreamSoundLocalized(IFunctionHandler *pH)
{	
	if (pH->GetParamCount()<1 || pH->GetParamCount()>2)
	{  
		m_pScriptSystem->RaiseError("System.LoadStreamSound wrong number of arguments"); 
		return pH->EndFunctionNull(); 
	};

	const char *sSound;
	//	int nID;
	ISound *pSound;
	pH->GetParam(1,sSound);

	int nFlags=0;
	if (pH->GetParamCount()>1) 
		pH->GetParam(2, nFlags);

	string szPath;

	ICVar *g_language = m_pSystem->GetIConsole()->GetCVar("g_language");
	assert(g_language);

	szPath = "LANGUAGES/";
	szPath += g_language->GetString();
	szPath += "/";
	szPath += sSound;

	if (m_pSoundSystem)
	{
		pSound=m_pSoundSystem->LoadSound(szPath.c_str(),FLAG_SOUND_STREAM | nFlags);
		if (pSound)
		{
			pSound->AddRef();
			USER_DATA ud=m_pScriptSystem->CreateUserData((INT_PTR)pSound,USER_DATA_SOUND);	//AMD Port
			return pH->EndFunction(ud);
		}
	}else
	{
		m_pScriptSystem->RaiseError("Sound.LoadStreamSound error loading %s",szPath.c_str());
	}
	return pH->EndFunctionNull();

}
*/
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Start playing a sound, loops is if enabled
	@param nID ID of the sound
	@see CScriptObjectSound::SetSoundLoop
*/
int CScriptObjectSound::PlaySound(IFunctionHandler *pH)
{
	int nCookie=0;
	float fVolumeScale=1.0f;
	ISound *pSound=NULL;

	if(pH->GetParamUDVal(1,(USER_DATA &)pSound,nCookie) && pSound && (nCookie==USER_DATA_SOUND))	//AMD Port
	{
		if (pH->GetParamCount()>1) 			
		{		
			if(!pH->GetParam(2,fVolumeScale))
			{
				fVolumeScale=1.0f;
			}
		}

		pSound->Play(fVolumeScale);
	}
	else
	{
		if(m_pSoundSystem)
			m_pScriptSystem->RaiseError("PlaySound NULL SOUND!!");
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::PlaySoundFadeUnderwater(IFunctionHandler *pH)
{
	int nCookie=0;
	float fVolumeScale=1.0f;
	ISound *pSound=NULL;	

	if(pH->GetParamUDVal(1,(USER_DATA &)pSound,nCookie) && pSound && (nCookie==USER_DATA_SOUND))	//AMD Port
	{		
		if (pH->GetParamCount()>1) 			
		{		
			if (!pH->GetParam(2,fVolumeScale))			
				fVolumeScale=1.0f;			
		}

		pSound->PlayFadeUnderwater(fVolumeScale);
	}
	else
	{
		if(m_pSoundSystem)
			m_pScriptSystem->RaiseError("PlaySound NULL SOUND!!");
	}
	return pH->EndFunction();
}

/*! Set max sound hearable distance
	@param nID ID of the sound
	@param fSoundDistance max sound distance, in meters
*/
/////////////////////////////////////////////////////////////////////////////////
/*int CScriptObjectSound::SetMaxSoundDistance(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int		nCookie=0;
	float fDistance=500;
	ISound *pSound=NULL;
	pH->GetParamUDVal(1,(int &)pSound,nCookie);

	pH->GetParam(2,fDistance);

	if(pSound && (nCookie==USER_DATA_SOUND))
		pSound->SetMaxSoundDistance(fDistance);

	return pH->EndFunction();
}*/

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Change the volume of a sound
	@param nID ID of the sound
	@param iVolume volume between 0 and 100
*/
int CScriptObjectSound::SetSoundVolume(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	int iVolume=0;
	ISound *pSound=NULL;
	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);

	pH->GetParam(2,iVolume);


	if (pSound && (nCookie==USER_DATA_SOUND))
	{
		if (iVolume<0)
			iVolume=0;
		pSound->SetVolume(iVolume);
	}

	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Change the looping status of a looped sound
	@param nID ID of the sound
	@param nFlag 1/0 to enable/disable looping
*/
int CScriptObjectSound::SetSoundLoop(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	int nFlag=0;
	ISound *pSound=NULL;
	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port

	pH->GetParam(2,nFlag);

	if (pSound && (nCookie==USER_DATA_SOUND))
	{
		//pSound->SetLooping(nFlag?true:false);
		pSound->SetLoopMode(nFlag?true:false);
	}

	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Changes the pitch of a sound sound
	@param nID ID of the sound
	@param nFrequency Frequency, value range is between 0 and 1000
*/
int CScriptObjectSound::SetSoundFrequency(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	int nFrequency=0;
	ISound *pSound=NULL;


	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port
	pH->GetParam(2,nFrequency);


  if (pSound && (nCookie==USER_DATA_SOUND))
  {
  	pSound->SetPitch(nFrequency);
  }
  
	return pH->EndFunction();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::SetSoundPitching(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	float fPitching=0;
	ISound *pSound=NULL;


	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port
	pH->GetParam(2,fPitching);


	if (pSound && (nCookie==USER_DATA_SOUND))
	{
		pSound->SetPitching(fPitching);
	}

	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::GetListener(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);

	CScriptObjectVector oVecParam1(m_pScriptSystem, true);
	CScriptObjectVector oVecParam2(m_pScriptSystem, true);
	CScriptObjectVector oVecParam3(m_pScriptSystem, true);

	pH->GetParam(1, *oVecParam1);
	pH->GetParam(2, *oVecParam2);
	pH->GetParam(3, *oVecParam3);

	return (pH->EndFunction());
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Stop playing a sound
	@param nID ID of the sound
*/
int CScriptObjectSound::StopSound(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int nCookie=0;
	ISound *pSound=NULL;
	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port

  if (pSound && (nCookie==USER_DATA_SOUND))
  {
  	pSound->Stop();
  }
  
	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Change the position of a 3D sound
	@param nID ID of the sound
	@param v3d Three component vector cotaining the position
	@see CScriptObjectSound::Load3DSound
*/
int CScriptObjectSound::SetSoundPosition(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	ISound *pSound=NULL;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3 v3d;

	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port

	pH->GetParam(2,*oVec);
	v3d=oVec.Get();

	if (pSound && (nCookie==USER_DATA_SOUND))
	{
		// Prevent script from changing the position of relative sounds
		if (pSound->IsRelative())
			return pH->EndFunction();

		pSound->SetPosition(v3d);
	}

	return pH->EndFunction();
}
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/*! Change the velocity of a sound
	@param nID ID of the sound
	@param v3d Three component vector containing the velocity for each axis
*/
int CScriptObjectSound::SetSoundSpeed(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	ISound *pSound;
	CScriptObjectVector oVec(m_pScriptSystem,true);
	Vec3 v3d;
	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port
	pH->GetParam(2,*oVec);
	v3d=oVec.Get();


  if (pSound && (nCookie==USER_DATA_SOUND))
  {
  	pSound->SetVelocity(v3d);
  }
  
	return pH->EndFunction();
}

/*! Set distance attenuation of a sound
	@param iSoundID ID of the sound
	@param fMinDist Minimum distance, normally 0
	@param fMaxDist Maximum distance at which the sound can be heared
*/
int CScriptObjectSound::SetMinMaxDistance(IFunctionHandler *pH)
{
	int nCookie=0;
	float fMinDist;
	float fMaxDist;
	ISound *pISound = NULL;

	CHECK_PARAMETERS(3);


	pH->GetParamUDVal(1,(INT_PTR &)pISound,nCookie);	//AMD Port

	pH->GetParam(2, fMinDist);
	pH->GetParam(3, fMaxDist);

	if(fMinDist<1)
		fMinDist=1;

  if (pISound && (nCookie==USER_DATA_SOUND))
  		pISound->SetMinMaxDistance(fMinDist, fMaxDist);
  
	return pH->EndFunction();
}

/*!
*/
int CScriptObjectSound::SetLoopPoints(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(3);

	int nID = 0, iLoopPt1, iLoopPt2;
	ISound *pISound = NULL;
	int nCookie=0;
	pH->GetParamUDVal(1,(INT_PTR &)pISound,nCookie);	//AMD Port
	pH->GetParam(2, iLoopPt1);
	pH->GetParam(3, iLoopPt2);
	if (pISound && (nCookie==USER_DATA_SOUND))
		pISound->SetLoopPoints(iLoopPt1, iLoopPt2);
	

	return pH->EndFunction();
}

/*!
*/
/*
int CScriptObjectSound::AddSoundFlags(IFunctionHandler *pH)
{
	int iFlags;
	int nCookie=0;
	ISound *pISound = NULL;
	CHECK_PARAMETERS(2);

	pH->GetParamUDVal(1,(int &)pISound,nCookie);
	pH->GetParam(2, iFlags);

	if (pISound && (nCookie==USER_DATA_SOUND))
		pISound->AddFlags(iFlags);
	 	
	return pH->EndFunction();
}
*/

int CScriptObjectSound::SetMasterVolumeScale(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	float fScale=1;
	if(pH->GetParam(1, fScale) && m_pSoundSystem)
		m_pSoundSystem->SetMasterVolumeScale(fScale);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::AddToScaleGroup(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nGroup;
	ISound *pSound=NULL;
	int nCookie=0;
	pH->GetParamUDVal(1, (INT_PTR&)pSound, nCookie);	//AMD Port
	pH->GetParam(2, nGroup);
	if (pSound && (nCookie==USER_DATA_SOUND))
		pSound->AddToScaleGroup(nGroup);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::RemoveFromScaleGroup(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nGroup;
	ISound *pSound=NULL;
	int nCookie=0;
	pH->GetParamUDVal(1, (INT_PTR&)pSound, nCookie);	//AMD Port
	pH->GetParam(2, nGroup);
	if (pSound && (nCookie==USER_DATA_SOUND))
		pSound->RemoveFromScaleGroup(nGroup);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::SetGroupScale(IFunctionHandler *pH)
{
	if(!m_pSoundSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(2);
	int nGroup;
	float fScale;
	pH->GetParam(1, nGroup);
	pH->GetParam(2, fScale);

	return pH->EndFunction(m_pSoundSystem->SetGroupScale(nGroup, fScale));
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::SetSoundProperties(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	float fFadingValue = 1;
	ISound *pSound=NULL;
	int nCookie=0;

	pH->GetParamUDVal(1,(INT_PTR &)pSound,nCookie);		//AMD Port
	pH->GetParam(2,fFadingValue);
	
	if (pSound && (nCookie==USER_DATA_SOUND))
		pSound->SetSoundProperties(fFadingValue);

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::FXEnable(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);

	int nCookie=0;
	ISound *pSound=NULL;
	int nEffectNumber;

	pH->GetParamUDVal(1, (INT_PTR&)pSound, nCookie);	//AMD Port
	pH->GetParam(2,nEffectNumber);
	if (pSound && (nCookie==USER_DATA_SOUND))
		pSound->FXEnable(nEffectNumber);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::SetFXSetParamEQ(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);

	int nCookie=0;
	ISound *pSound=NULL;
	float fCenter;
	float fBandwidth;
	float fGain;

	pH->GetParamUDVal(1, (INT_PTR&)pSound, nCookie);	//AMD Port
	pH->GetParam(2,fCenter);
	pH->GetParam(3,fBandwidth);
	pH->GetParam(4,fGain);
	if (pSound && (nCookie==USER_DATA_SOUND))
		pSound->FXSetParamEQ(fCenter, fBandwidth, fGain);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::SetDirectionalAttenuation(IFunctionHandler *pH)
{
	if(!m_pSoundSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(3);
	CScriptObjectVector oVec(m_pScriptSystem, true);
	Vec3 Pos;
	Vec3 Dir;
	float fConeInRadians;
	pH->GetParam(1, oVec);
	Pos=oVec.Get();
	pH->GetParam(2, oVec);
	Dir=oVec.Get();

	Dir=ConvertToRadAngles(Dir);
	if (!pH->GetParam(3, fConeInRadians))
		fConeInRadians=0.0f;
	else
		fConeInRadians*=0.5f;	// we need the half-angle

	m_pSoundSystem->CalcDirectionalAttenuation(Pos, Dir, fConeInRadians);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::GetDirectionalAttenuationMaxScale(IFunctionHandler *pH)
{
	if(!m_pSoundSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(0);
	return pH->EndFunction(m_pSoundSystem->GetDirectionalAttenuationMaxScale());
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::LoadMusic(IFunctionHandler *pH)
{
	bool bRes = true;
	/*
	CHECK_PARAMETERS(1);
	const char *pszFilename;
	pH->GetParam(1, pszFilename);
	bRes=m_pSystem->GetIMusicSystem()->LoadFromXML(pszFilename,true);
	if (!bRes)
		m_pSystem->GetILog()->Log("Unable to load music from %s !", pszFilename);
		*/
	return pH->EndFunction(bRes);
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::UnloadMusic(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(0);
	m_pMusicSystem->Unload();
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::SetMusicTheme(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	if (pH->GetParamCount()<1)
	{
		CHECK_PARAMETERS(1);
	}
	const char *pszTheme;
	bool bOverride=false;
	if (!pH->GetParam(1, pszTheme))
		return pH->EndFunction(false);
	if (pH->GetParamCount()>=2)
		pH->GetParam(2, bOverride);
	bool bRes=m_pMusicSystem->SetTheme(pszTheme, bOverride);
	if (!bRes)
		m_pSystem->GetILog()->Log("Unable to set music-theme \"%s\" !", pszTheme);
	return pH->EndFunction(bRes);
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::ResetMusicThemeOverride(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(0);
	bool bRes=m_pMusicSystem->ResetThemeOverride();
	if (!bRes)
		m_pSystem->GetILog()->Log("Unable to reset music-theme-override !");
	return pH->EndFunction(bRes);
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::SetMusicMood(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(1);
	const char *pszMood;
	if (!pH->GetParam(1, pszMood))
		return pH->EndFunction(false);
	bool bRes=m_pMusicSystem->SetMood(pszMood);
	if (!bRes)
		m_pSystem->GetILog()->Log("Unable to set music-mood \"%s\" !", pszMood);
	return pH->EndFunction(bRes);
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::SetDefaultMusicMood(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(1);
	const char *pszMood;
	if (!pH->GetParam(1, pszMood))
		return pH->EndFunction(false);
	bool bRes=m_pMusicSystem->SetDefaultMood(pszMood);
	if (!bRes)
		m_pSystem->GetILog()->Log("Unable to set default music-mood \"%s\" !", pszMood);
	return pH->EndFunction(bRes);
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::GetMusicThemes(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(0);
	IStringItVec *pVec=m_pMusicSystem->GetThemes();
	if (!pVec)
		return pH->EndFunctionNull();
	_SmartScriptObject pTable(m_pScriptSystem);
	int i=1;
	while (!pVec->IsEnd())
	{
		pTable->SetAt(i++, pVec->Next() );
	}
	pVec->Release();
	return pH->EndFunction(pTable);
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::GetMusicMoods(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(1);
	const char *pszTheme;
	pH->GetParam(1, pszTheme);
	IStringItVec *pVec=m_pMusicSystem->GetMoods(pszTheme);
	if (!pVec)
		return pH->EndFunctionNull();
	_SmartScriptObject pTable(m_pScriptSystem);
	int i=1;
	while (!pVec->IsEnd())
	{
		pTable->SetAt(i++, pVec->Next());
	}
	pVec->Release();
	return pH->EndFunction(pTable);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::AddMusicMoodEvent(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunctionNull();

	CHECK_PARAMETERS(2);
	const char *pszMood;	// name of mood
	float fTimeout=1.0f;	// minimum time this mood-event must be set in order to actually switch to the mood
	pH->GetParam(1, pszMood);
	pH->GetParam(2, fTimeout);

	return pH->EndFunction(m_pMusicSystem->AddMusicMoodEvent(pszMood, fTimeout));
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::IsInMusicTheme(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunctionNull();

	CHECK_PARAMETERS(1);
	const char *pszTheme;
	pH->GetParam(1, pszTheme);
	SMusicSystemStatus *pStatus=m_pMusicSystem->GetStatus();
	if (pStatus)
		return pH->EndFunction(stricmp(pStatus->sTheme.c_str(), pszTheme)==0);
	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////////
int	CScriptObjectSound::IsInMusicMood(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(1);
	const char *pszMood;
	pH->GetParam(1, pszMood);
	SMusicSystemStatus *pStatus=m_pMusicSystem->GetStatus();
	if (pStatus)
		return pH->EndFunction(stricmp(pStatus->sMood.c_str(), pszMood)==0);
	return pH->EndFunction();
}

int CScriptObjectSound::GetSoundLength(IFunctionHandler * pH)
{
	CHECK_PARAMETERS(1);

	int nCookie=0;
	ISound *pSound=NULL;

	pH->GetParamUDVal(1, (INT_PTR&)pSound, nCookie);	//AMD Port

	if (pSound && (nCookie==USER_DATA_SOUND))
		return pH->EndFunction( (float)pSound->GetLengthMs()/1000.f);

	return pH->EndFunctionNull();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::GetMusicStatus(IFunctionHandler *pH)
{
	if(!m_pMusicSystem)
		return pH->EndFunction();

	CHECK_PARAMETERS(0);
	SMusicSystemStatus *pStatus=m_pMusicSystem->GetStatus();
	ILog *pLog=m_pSystem->GetILog();
	assert(pLog);
	pLog->LogToConsole("--- MusicSystem Status Info ---");
	pLog->LogToConsole("  Streaming: %s", pStatus->bPlaying ? "Yes" : "No");
	pLog->LogToConsole("  Theme: %s", pStatus->sTheme.c_str());
	pLog->LogToConsole("  Mood: %s", pStatus->sMood.c_str());
	pLog->LogToConsole("  Playing patterns:");
	for (TPatternStatusVecIt It=pStatus->m_vecPlayingPatterns.begin();It!=pStatus->m_vecPlayingPatterns.end();++It)
	{
		SPlayingPatternsStatus &PatternStatus=(*It);
		pLog->LogToConsole("    %s (Layer: %s; Volume: %s)", PatternStatus.sName.c_str(), (PatternStatus.nLayer==MUSICLAYER_MAIN) ? "Main" : ((PatternStatus.nLayer==MUSICLAYER_RHYTHMIC) ? "Rhythmic" : "Incidental"), PatternStatus.nVolume);
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectSound::SetSoundRatio(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	int nCookie=0;
	ISound *pSound=NULL;
	float fRatio;
	pH->GetParamUDVal(1, (INT_PTR&)pSound, nCookie);	//AMD Port
	pH->GetParam(2, fRatio);
	if (pSound && (nCookie==USER_DATA_SOUND))
		pSound->SetRatio(fRatio);
	return pH->EndFunction();
}
