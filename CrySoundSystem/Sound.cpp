//////////////////////////////////////////////////////////////////////
//
//  Crytek (C) 2001
//
//  CrySound Source Code
// 
//  File: Sound.cpp
//  Description: ISound interface implementation.
// 
//  History: 
//	-June 06,2001:Implemented by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifndef _XBOX 
#include "Sound.h"
#include <CrySizer.h>
#include <Cry_Camera.h>
#include "SoundSystem.h"
#include <ICOnsole.h>
#include <ISystem.h>
#include <ITimer.h>
#include <I3dEngine.h> //needed to check if the listener is in indoor or outdoor

#pragma warning(disable:4003)	// warning C4003: not enough actual parameters for macro 'CHECK_LOADED'
 
#define CRYSOUND_MAXDIST	10000.0f

//////////////////////////////////////////////////////////////////////////

#define CHECK_LOADED(_func, _retval)	if (!m_pSound->Loaded()) { TRACE("%s(%d) Warning: %s called without a sound (%s) being fully loaded !", __FILE__, __LINE__, #_func, GetName()); return _retval; }

int CSound::m_PlayingChannels = 0;

//////////////////////////////////////////////////////////////////////
CSound::CSound(CSoundSystem *pSSys,const char *szFile)
{
	ASSERT(pSSys);
	ASSERT(pSSys->m_pISystem);
	m_pSSys = pSSys;
	m_pTimer=pSSys->m_pISystem->GetITimer();
//	m_refCount = 0;

	//SetName( "Sound not loaded" );
//	m_nId = soundId;
	m_nChannel=0;
	m_bPlaying=false;
	m_position(0,0,0);
	m_speed(0,0,0);
	m_orient(0,0,0);

	m_State=eSoundState_None;
	m_fLastPlayTime=0.0f;
	
	m_nChannel=CS_FREE;

	m_nSoundScaleGroups=SETSCALEBIT(SOUNDSCALE_MASTER) | SETSCALEBIT(SOUNDSCALE_SCALEABLE) | SETSCALEBIT(SOUNDSCALE_DEAFNESS) | SETSCALEBIT(SOUNDSCALE_MISSIONHINT);
	m_nStartOffset=0;
	m_nVolume=-1;
	m_nPlayingVolume=-1;
	m_nPan=127;
	m_fPitching=-1;
	m_nCurrPitch=0;
	//m_nBaseFreq=44100;
	m_nRelativeFreq=1000;
	m_nFxChannel=-1;
	m_bLoop=false;
	m_fRatio=1.0f;

	//m_nLastId=0;
	//m_nFadeType=0;
	m_fMaxRadius2=m_fMinRadius2=m_fDiffRadius2=0;
	m_fPreloadRadius2=0;
	m_nMaxVolume=255;
	m_fMinDist=m_fMaxDist=1;
	m_fSoundLengthSec = 0;

//	m_fMaxSoundDistance2=500*500;
	m_refCount=0;
	m_pArea=NULL;
	m_fFadingValue=1.0f; ///5.0f; // by default 
	//m_fCurrentFade=1.0f; 	
	m_fCurrentFade=1.0f; 

	m_nSoundPriority=0;

	m_bPlayAfterLoad=false;
	m_fPostLoadRatio=0;
	m_bPostLoadForceActiveState=false;
	m_bPostLoadSetRatio=false;
	m_bAutoStop = false;
	m_bAlreadyLoaded = false;

	SetAttrib(255, 1.0f, 127);

	//Timur: Never addref on creation.
	//AddRef();
	//SetName(szFile);	
}

//////////////////////////////////////////////////////////////////////
CSound::~CSound()
{
	if (m_pSound)
		m_pSound->RemoveFromLoadReqList(this);
	//stop the sound
	if (m_nChannel>=0)  
		Stop();

	if (m_pSSys->m_pCVarDebugSound->GetIVal()==1)
	{
		m_pSSys->m_pILog->Log("<Sound> Destroying sound %s \n",GetName());		
	}

	m_pSSys->RemoveReference(this);
}

//////////////////////////////////////////////////////////////////////
int CSound::Release()
{
	//reset the reference
	int ref=0;
	if((ref=--m_refCount)<=0)
	{
#if defined(WIN64)// && defined(_DEBUG)
		// Win64 has a special variable to disable deleting all the sounds
		ICVar* pVar = m_pSSys->GetSystem()->GetIConsole()->GetCVar("s_EnableRelease");
		if (!pVar || pVar->GetIVal())
#endif
		delete this;
	}
	return (ref);
};

//load the sound for real
//////////////////////////////////////////////////////////////////////
bool CSound::Preload()
{
	if (!m_pSSys->IsEnabled())
	{
		OnEvent( SOUND_EVENT_ON_LOADED );
		return true;
	}
	return m_pSound->Load(m_bLoop, this);
}

//////////////////////////////////////////////////////////////////////////
void CSound::SetSoundPriority(unsigned char nSoundPriority)
{
	m_nSoundPriority=nSoundPriority;

	if (nSoundPriority==255)
	{
		// keep in memory all sounds with highest priority
		// Not needed.
		//m_nFlags|=FLAG_SOUND_LOAD_SYNCHRONOUSLY;
		//Preload(); 
	}	
}
/*
//////////////////////////////////////////////////////////////////////
void CSound::SetMaxSoundDistance(float fMaxSoundDistance)
{
	//keep it squared
	m_fMaxSoundDistance2=fMaxSoundDistance*fMaxSoundDistance;
}*/

//calc the sound volume takiing into account gaem sound attenuation,
//reduction ratio, sound system sfx volume, master volume (concentration feature)
//////////////////////////////////////////////////////////////////////
int CSound::CalcSoundVolume(int nSoundValue,float fRatio)
{
	//FRAME_PROFILER( "CSound:CalcSoundVolume",m_pSSys->GetSystem(),PROFILE_SOUND );

	int nVolume;

	for (int i=0;i<MAX_SOUNDSCALE_GROUPS;i++)
	{
		if (m_nSoundScaleGroups & SETSCALEBIT(i))
			fRatio*=m_pSSys->m_fSoundScale[i];
	}

/*	if (m_pSound->m_btType==SoundBuffer::btSTREAM)
	{
		//straming sounds use music volume
		nVolume=(int)((float)nSoundValue*fRatio*m_pSSys->m_fMusicVolume);
	}
	else 
	{*/
	//if (m_nFlags & FLAG_SOUND_MUSIC)
	//	nVolume=(int)((((float)nSoundValue*m_pSSys->m_fMusicVolume)*fRatio));
	//else			
		nVolume=(int)((((float)nSoundValue*m_pSSys->m_fSFXVolume)*fRatio));
//	}

	return (nVolume);
}

//////////////////////////////////////////////////////////////////////
bool CSound::IsPlaying()
{
	//if (m_bPlaying && m_bLoop)
	if (m_bPlaying)
		return true;
	return false;

	//return IsPlayingOnChannel();
}

//////////////////////////////////////////////////////////////////////
bool CSound::IsPlayingOnChannel()
{
	if (m_nChannel == CS_FREE)
		return false;
	GUARD_HEAP;
	signed char bRes=CS_IsPlaying(m_nChannel);

	if (bRes==TRUE)
	{
		//m_pSSys->m_pILog->LogToConsole("channel %d IS playing!!",m_nChannel);
		return (true);
	}

	//m_pSSys->m_pILog->LogToConsole("channel %d,not playing!!",m_nChannel);
	return (false);	
}

//////////////////////////////////////////////////////////////////////////
bool CSound::IsPlayingVirtual()
{
	return m_bPlaying && m_bLoop;
}
 
//////////////////////////////////////////////////////////////////////
void CSound::PlayFadeUnderwater(float fRatio, bool bForceActiveState, bool bSetRatio)
{
	m_pSSys->m_lstFadeUnderwaterSounds.insert(this);
	Play(fRatio,bForceActiveState,bSetRatio);
	m_nFlags|=FLAG_SOUND_FADE_OUT_UNDERWATER;
}

//////////////////////////////////////////////////////////////////////
void CSound::Play(float fRatio, bool bForceActiveState, bool bSetRatio)
{
	GUARD_HEAP;
	FUNCTION_PROFILER( m_pSSys->GetSystem(),PROFILE_SOUND );

	if (!m_pSSys->IsEnabled())
	{
		// Simulate that sound is played for disabled sound system.
		OnEvent( SOUND_EVENT_ON_LOADED );
		OnEvent( SOUND_EVENT_ON_PLAY );
		OnEvent( SOUND_EVENT_ON_STOP );
		return;
	}

	float currTime = m_pTimer->GetCurrTime();
	float timeSinceLastPlay = currTime - m_fLastPlayTime;

	if (bSetRatio)
		m_fRatio = fRatio;

	//////////////////////////////////////////////////////////////////////////
	// If sound is already playing, skip until it stops.
	// overwise it creates alot of overhead and channel leaks.
	//////////////////////////////////////////////////////////////////////////
	if (IsPlayingOnChannel())
	{
		if (!m_bLoop)
		{
			// For not looping sounds stop previous if after minimal repeat timeout.
			if (m_pSSys->m_pCVarMinRepeatSoundTimeout->GetIVal() > timeSinceLastPlay*1000.0f)
			{
				return;
			}
		}
		else {
			// For looping sounds ignore, repeated play.
			return;
		}
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// If sound is already allocated fmod channel (may not even play it), it must free it before.
	if (IsUsingChannel())
	{
		FreeChannel();
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Sounds with lower priority then current minimum set by sound system are ignored.
	if (m_nSoundPriority < m_pSSys->GetMinSoundPriority())
		return;
	//////////////////////////////////////////////////////////////////////////

	bool bStartSound=true;
	if (m_nFlags & FLAG_SOUND_OUTDOOR)
	{
		if (m_pSSys->m_bInside)
			bStartSound=false;
	}
	
	if (m_nFlags & FLAG_SOUND_INDOOR)		
	{	
		if (!m_pSSys->m_bInside)
			bStartSound=false;
	}

	// if there are already too many sounds playing, this one
	// should not be started
	bool bCanPlay = ((int)(m_pSSys->m_lstSoundSpotsActive.size())<m_pSSys->m_pCVarMaxSoundSpots->GetIVal());

	//////////////////////////////////////////////////////////////////////////
	// For Not looping sounds too far away, stop immidiatly not even preload sound if too far.
	//////////////////////////////////////////////////////////////////////////
	if (!m_bLoop)
	{
		if (!bStartSound || !bCanPlay)
		{
			// Dont try to start non looping sounds at all.
			return;
		}
		if (m_nFlags & (FLAG_SOUND_3D|FLAG_SOUND_RADIUS))
		{
			if (!m_pSSys->m_bValidPos)
			{
				return; //too far
			}
			//check the distance from the listener
			float fDist2 = GetSquaredDistance(m_pSSys->m_cCam.GetPos(),m_RealPos);//position);

			if (fDist2 > m_fMaxRadius2)
			{
				return; //too far
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	//load the sound for real the first time is played
	if (!Preload())
		return; //the sound buffer cannot be loaded

	// Preloading is also playing.
	m_bPlaying = true;

	bool bAllocatedChannel = false;

	if (m_pSound->Loading())
	{

		if (m_pSSys->m_pCVarDebugSound->GetIVal()==3)
			m_pSSys->m_pILog->Log("<Sound> Warning: %s, Play() called before loading was finished", GetName());
		
		// save the state, will be played later
		m_bPlayAfterLoad=true;
		m_fPostLoadRatio=fRatio;
		m_bPostLoadForceActiveState=bForceActiveState;
		m_bPostLoadSetRatio=bSetRatio;
		return; 

		/*
		float fTime;
		if (m_pSSys->m_pCVarDebugSound->GetIVal())
			fTime=m_pTimer->GetAsyncCurTime();
		TRACE("INFO: Synchroneous reading of %s due to Play() call before loading was finished.", GetName());
		if (!m_pSound->WaitForLoad())
			return;
		if (m_pSSys->m_pCVarDebugSound->GetIVal())
			m_pSSys->m_pILog->Log("Warning: Synchroneous reading of %s due to Play() call before loading was finished stalled for %1.3f milliseconds !", GetName(), (m_pTimer->GetAsyncCurTime()-fTime)*1000.0f);
		*/
	} 
	//CHECK_LOADED(Play);
	//UpdatePosition();
	bool bStarted=false;

	int nVolume=CalcSoundVolume(m_nVolume,fRatio);

	if (bStartSound)
	{
		switch (m_pSound->GetType())
		{
			case btSTREAM:
				if ((!m_bLoop) || (!IsPlayingOnChannel()))
				{
					if (bCanPlay)
					{
						m_nChannel=CS_Stream_PlayEx(CS_FREE, m_pSound->GetStream(), NULL, true);
						m_PlayingChannels++;
						m_fChannelPlayTime = currTime;
						m_fSoundLengthSec = GetLengthMs()/1000.0f;
						bAllocatedChannel = true;
					}
					bStarted=true;
				}
				SetLoopMode(m_bLoop);
				break;
			case btSAMPLE:
				{										
					if (m_nFlags & FLAG_SOUND_3D)
					{
						if (!m_pSSys->m_bValidPos)
						{
							if (m_bLoop)
								bCanPlay = false;
							else
								return;
						}
												
						//check the distance from the listener
						float fDist=GetSquaredDistance(m_pSSys->m_cCam.GetPos(),m_RealPos);//position);

						if (fDist>m_fMaxRadius2)
						{
							if (!m_bLoop)
							{
								FreeChannel(); //free a channel
								return; //too far
							}
							bCanPlay = false;
						}
						
						//min max distance and loop mode must be set before 'cos it operates
						//on a per-sound basis
						SetMinMaxDistance(m_fMinDist,m_fMaxDist);
						SetLoopMode(m_bLoop);

						FRAME_PROFILER( "CSound:CS_PlaySoundEX",m_pSSys->GetSystem(),PROFILE_SOUND );						
						if ((!m_bLoop) || (!IsPlayingOnChannel()))
						{						
							if (bCanPlay)
							{
								m_nChannel=CS_PlaySoundEx(CS_FREE, m_pSound->GetSample(), NULL, true);//,m_pSSys->GetDSPUnitFilter(),FALSE);
								m_PlayingChannels++;
								m_fChannelPlayTime = currTime;
								m_fSoundLengthSec = GetLengthMs()/1000.0f;
								bAllocatedChannel = true;
							}
							bStarted=true;
						}
						SetPosition(m_position);
						//SetFrequency(m_nBaseFreq);
					}
					else			
					{
						//loop mode must be set before 'cos it operates
						//on a per-sound basis
						SetLoopMode(m_bLoop);
						if ((!m_bLoop) || (!IsPlayingOnChannel()))
						{
							if (bCanPlay)
							{
								m_nChannel=CS_PlaySoundEx(CS_FREE, m_pSound->GetSample(), NULL, true);//,m_pSSys->GetDSPUnitFilter(),FALSE);													
								m_PlayingChannels++;
								m_fChannelPlayTime = currTime;
								m_fSoundLengthSec = GetLengthMs()/1000.0f;
								bAllocatedChannel = true;
							}
							bStarted=true;
						}
						//SetFrequency(m_nRelativeFreq);			
					}
				}
				if (bStarted)
				{					
					if (m_fPitching>0.0f)
					{			
						float fRand=(float)rand()/(float)RAND_MAX;
						fRand=fRand*2.0f-1.0f;
						m_nCurrPitch=(int)(fRand*m_fPitching);
					}
					else
						m_nCurrPitch=0;

					SetFrequency(m_nRelativeFreq);
					if (m_nStartOffset)
						CS_SetCurrentPosition(m_nChannel, m_nStartOffset);
					m_nStartOffset=0;
				}
				break;
		}
	}
	
	//set volume (after we got the channel)
	ChangeVolume(nVolume);
	//set panning as well (after we got the channel)
	if (bCanPlay && m_nChannel!=-1)
	{
		CS_SetPan(m_nChannel, m_nPan);		
		CS_SetPriority(m_nChannel, m_nSoundPriority);
		if (bStarted)
			CS_SetPaused(m_nChannel,false);
	}

	bool bAddedToList = false;
	if (m_bLoop && bForceActiveState && (!m_pSound->LoadFailure()))
	{
		if (m_nFlags & FLAG_SOUND_ACTIVELIST)
		{		
			if (bCanPlay)
			{
				m_pSSys->SetSoundActiveState(this, eSoundState_Active);			
				bAddedToList = true;
			}
			else
			{
				// let's put it in the inactive list if can't be played
				// right now
				m_pSSys->SetSoundActiveState(this, eSoundState_Inactive);
				bAddedToList = true;
			}
		}
	}
	// If not looping sound it should be added to temporary auto stop sound list, 
	// to be stoped and free channel after it have been played.
	if (IsUsingChannel() && !m_bLoop)
	{
		m_bAutoStop = true;
		m_pSSys->RegisterAutoStopSound( this );
	}

	m_fLastPlayTime = currTime;
		
	if (bAllocatedChannel && m_pSSys->m_pCVarDebugSound->GetIVal()==1)
	{
		m_pSSys->m_pILog->Log("<Sound> Playing sound: %s,channel %d,volume=%d,pan=%d, priority=%d Snd:%X, TimeDiff:%.2f",GetName(),m_nChannel,m_nPlayingVolume,m_nPan,m_nSoundPriority,this,timeSinceLastPlay);
	}

	if (bAllocatedChannel)
	{
		OnEvent( SOUND_EVENT_ON_PLAY );
	}
}

//////////////////////////////////////////////////////////////////////////
void CSound::FreeChannel()
{
	if (!m_bPlaying || m_nChannel <= 0)
		return;
	m_nPlayingVolume = -1;
	if (m_pSound->Loaded())
	{ 
		switch (m_pSound->GetType())
		{
		case btSTREAM:
			CS_Stream_Stop(m_pSound->GetStream());
			m_PlayingChannels--;
			break;
		case btSAMPLE:
			CS_StopSound(m_nChannel);
			m_PlayingChannels--;
			break;
		}
	}
	m_fChannelPlayTime = -1;
	m_nChannel = CS_FREE;
}

//////////////////////////////////////////////////////////////////////
void CSound::Stop()
{
	FUNCTION_PROFILER( m_pSSys->GetSystem(),PROFILE_SOUND );
	GUARD_HEAP;
	// don't play after finishing with loading - if still loading yet...
	m_bPlayAfterLoad=false; 
	if (!m_bPlaying)
		return;

	bool bUsedChannel = IsUsingChannel();

	FreeChannel();

	m_bPlaying = false;
	m_nPlayingVolume = -1;
	if (bUsedChannel && (m_pSSys->m_pCVarDebugSound->GetIVal()==1))
		m_pSSys->m_pILog->Log("<Sound> Stop sound: %s",GetName());

	if (m_nFlags & FLAG_SOUND_FADE_OUT_UNDERWATER)
	{
		SoundListItor It=m_pSSys->m_lstFadeUnderwaterSounds.find(this);
		if (It!=m_pSSys->m_lstFadeUnderwaterSounds.end())
			m_pSSys->m_lstFadeUnderwaterSounds.erase(It);
	}
	if (bUsedChannel)
		OnEvent( SOUND_EVENT_ON_STOP );

	// Must be last.
	if (m_bAutoStop)
	{
		m_bAutoStop = false;
		// This can deallocate the sound.
		m_pSSys->UnregisterAutoStopSound( this );
	}
}

/*
//////////////////////////////////////////////////////////////////////
void CSound::SetName(const char *szName)
{
	if (szName)		
	{		
		m_strName = szName;
	}
	else
		m_strName = "";
}
*/

//////////////////////////////////////////////////////////////////////
const char *CSound::GetName()
{
	//return (m_strName.c_str());
	if (m_pSound!=NULL)
		return (m_pSound->GetName());
	return (m_pSSys->m_szEmptyName);
}

//////////////////////////////////////////////////////////////////////
const int CSound::GetId()
{
	return 0;//(m_nId);
}

//////////////////////////////////////////////////////////////////////
void CSound::SetLoopMode(bool bLoop)
{
	GUARD_HEAP;
	//FRAME_PROFILER( "CSound:SetLoopMode",m_pSSys->GetSystem(),PROFILE_SOUND );

	if (m_pSound->Loaded())	// && m_pSound->m_btType==SoundBuffer::btSAMPLE && m_pSound->m_data.m_sfx)
	{
		switch (m_pSound->GetType())
		{
			//case btSAMPLE:	CS_Sample_SetMode(m_pSound->GetSample(), bLoop ? CS_LOOP_NORMAL : CS_LOOP_OFF);	break;
      case btSAMPLE:	CS_Sample_SetMode(m_pSound->GetSample(), m_pSound->GetFModFlags(bLoop));	break;
			//case btSTREAM:	if (m_nChannel>=0) CS_SetLoopMode(m_nChannel, bLoop ? CS_LOOP_NORMAL : CS_LOOP_OFF); break;	// [Lennert] this call screws up ! Always set loopmode before playing the stream-sound !		
		}
	}
	if (bLoop)
		m_nFlags|=FLAG_SOUND_LOOP;
	else
		m_nFlags&=~FLAG_SOUND_LOOP;
	if ((m_nFlags & FLAG_SOUND_STREAM) && (m_bLoop!=bLoop))
	{	// this is kinda unfortunate, but we have to reload the stream when we the change loop-mode because changing it runtime doesnt seem to work...
		if (!m_pSound->NotLoaded())
		{
			m_pSound->AbortLoading();
			m_pSound->DestroyData();
			m_pSound->Load(bLoop, NULL);
		}
	}
	m_bLoop=bLoop;
}

//////////////////////////////////////////////////////////////////////
void CSound::SetPitching(float fPitching)
{
	m_fPitching = fPitching;
}

//////////////////////////////////////////////////////////////////////
void CSound::SetBaseFrequency(int nFreq)
{
	//m_nBaseFreq = nFreq;
}

//////////////////////////////////////////////////////////////////////
int	 CSound::GetBaseFrequency(int nFreq)
{
	if (m_pSound)
		return (m_pSound->GetBaseFreq());
	return (0);
}

//////////////////////////////////////////////////////////////////////
void CSound::SetPan(int nPan)
{
	if (m_nChannel>=0) 
	{
		if (nPan != m_nPan)
		{
			CS_SetPan(m_nChannel,nPan);
		}
	}
	m_nPan=nPan;
}

//////////////////////////////////////////////////////////////////////
void CSound::SetFrequency(int nFreq)
{
	//FRAME_PROFILER( "CSound:SetFrequency",m_pSSys->GetSystem(),PROFILE_SOUND );

	m_nRelativeFreq = nFreq;

	if ((m_nChannel>=0) && (m_pSound->GetType()==btSAMPLE))
	{
		float fFreq=(float)m_pSound->GetBaseFreq()*((float)(m_nRelativeFreq+m_nCurrPitch)/1000.0f);
		CS_SetFrequency(m_nChannel, (int)fFreq);
	}
}

//////////////////////////////////////////////////////////////////////
void CSound::SetMinMaxDistance(float fMinDist, float fMaxDist)
{
	//FRAME_PROFILER( "CSound:SetMinMaxDistance",m_pSSys->GetSystem(),PROFILE_SOUND );

	// increasing mindistnace makes it louder in 3d space	

	m_fMinDist=fMinDist;
	m_fMaxDist=fMaxDist;

	m_fMaxRadius2=fMaxDist*fMaxDist;
	m_fMinRadius2=fMinDist*fMinDist;
	float fPreloadRadius=fMaxDist+SOUND_PRELOAD_DISTANCE;
	m_fPreloadRadius2=fPreloadRadius*fPreloadRadius;

#ifdef _DEBUG
	if(fMinDist<0.1f)
	{
		m_pSSys->m_pILog->LogToConsole("SOUND[%s] min out of range %f",GetName(),fMinDist);
	}
	if(fMaxDist>1000000000.0f)
	{
		m_pSSys->m_pILog->LogToConsole("SOUND[%s] max out of range %f",GetName(),fMaxDist);
	}
#endif
	if (m_pSound->GetSample())
	{
		GUARD_HEAP;
		CS_Sample_SetMinMaxDistance(m_pSound->GetSample(), fMinDist, CRYSOUND_MAXDIST);//fMaxDist);
	}
}

//////////////////////////////////////////////////////////////////////
void CSound::SetAttrib(int nVolume, float fRatio, int nPan, int nFreq, bool bSetRatio)
{
	ASSERT(nFreq==1000);
	if (bSetRatio)
		m_fRatio=fRatio;
	m_nVolume=nVolume;

	if (m_nChannel>=0) 
	{
		ChangeVolume( CalcSoundVolume(m_nVolume,fRatio) );
		if (m_nPan != nPan)
		{
			CS_SetPan(m_nChannel,nPan);
		}
	}
	m_nPan=nPan;
}
	
//////////////////////////////////////////////////////////////////////
void CSound::SetAttrib(const Vec3d &pos, const Vec3d &speed)
{
  m_position=pos;
  m_speed=speed;
}

//////////////////////////////////////////////////////////////////////////
void CSound::SetRatio(float fRatio)
{
	m_fRatio=fRatio;
	ChangeVolume(CalcSoundVolume(m_nVolume,m_fRatio));
}

//////////////////////////////////////////////////////////////////////////
void CSound::UpdatePosition()
{
	SetPosition(m_position);
}

//////////////////////////////////////////////////////////////////////
void CSound::SetPosition(const Vec3d &pos)
{

	m_position=pos; 
	m_RealPos=m_position;
	
	if (!(m_nFlags & FLAG_SOUND_3D))
		return;

	/*
	if (m_pSSys->m_pCVarDebugSound->GetIVal()==1)
	{
		m_pSSys->m_pILog->Log("Sound %s, pos+%f,%f,%f",m_strName.c_str(),m_position.x,m_position.y,m_position.z);
	}
	*/

	float fPos[3];

	fPos[0]=m_position.x;
	fPos[1]=m_position.z;
	fPos[2]=m_position.y;   	

	if ((m_pSSys->m_fDirAttCone>0.0f) && (m_pSSys->m_fDirAttCone<1.0f))
	{
		Vec3d DirToPlayer=m_position-m_pSSys->m_DirAttPos;
		DirToPlayer.Normalize();
		float fScale=((1.0f-(DirToPlayer*m_pSSys->m_DirAttDir))/(1.0f-m_pSSys->m_fDirAttCone));
		if ((fScale>0.0f) && (fScale<=1.0f))	// inside cone
		{
			if ((1.0f-fScale)>m_pSSys->m_fDirAttMaxScale)
				m_pSSys->m_fDirAttMaxScale=1.0f-fScale;
			m_RealPos=m_position*fScale+(m_pSSys->m_DirAttPos+(m_pSSys->m_DirAttDir*0.1f))*(1.0f-fScale);
			fPos[0]=m_RealPos.x;
			fPos[1]=m_RealPos.z;
			fPos[2]=m_RealPos.y;   	
			//TRACE("Sound-Proj: %s, %1.3f, %1.3f, %1.3f, (%5.3f, %5.3f, %5.3f)", m_strName.c_str(), fScale, DirToPlayer*m_pSSys->m_DirAttDir, m_pSSys->m_fDirAttCone, m_RealPos.x, m_RealPos.y, m_RealPos.z);
		}
	}
	
	if (m_pSSys->m_pCVarDopplerEnable->GetIVal() && (m_nFlags & FLAG_SOUND_DOPPLER))
	{
		Vec3d vVel=(pos-m_position);
		float fTimeDelta=m_pSSys->m_pISystem->GetITimer()->GetFrameTime();

		vVel=vVel*(m_pSSys->m_pCVarDopplerEnable->GetFVal()/fTimeDelta);

		float fVel[3];
		fVel[0]=vVel.x;
		fVel[1]=vVel.z;
		fVel[2]=vVel.y;        

		GUARD_HEAP;
		CS_3D_SetAttributes(m_nChannel, fPos, fVel);
	}	
	else
	{
		GUARD_HEAP;
		//m_pSSys->m_pILog->LogToConsole("Setting sound pos for channel %d",m_nChannel);
		CS_3D_SetAttributes(m_nChannel, fPos, NULL);
	}

	// check if the sound must be considered 
	// for sound occlusion 
	if (m_nFlags & FLAG_SOUND_OCCLUSION)
	{		
		//check where the position is 	
		I3DEngine			*p3dEngine=m_pSSys->m_pISystem->GetI3DEngine();

		if (p3dEngine)		
			m_pArea=p3dEngine->GetVisAreaFromPos(pos);		
	}
}

//////////////////////////////////////////////////////////////////////
const bool CSound::GetPosition(Vec3d &vPos)
{
	if (m_nFlags & FLAG_SOUND_3D)
	{
		vPos=m_position;
		return (true);
	}

	return (false);
}

//////////////////////////////////////////////////////////////////////
void CSound::SetVelocity(const Vec3d &speed)
{  
  m_speed=speed;  
}

//////////////////////////////////////////////////////////////////////
void CSound::SetDirection(const Vec3d &vDir)
{
	m_orient=vDir;
}

//////////////////////////////////////////////////////////////////////
void CSound::ChangeVolume(int nVolume)
{
	if (m_nChannel>=0 && nVolume != m_nPlayingVolume)
//	if (m_nChannel>=0)
	{
		CS_SetVolume(m_nChannel, nVolume);
		m_nPlayingVolume = nVolume;
	}
}

//////////////////////////////////////////////////////////////////////
void CSound::SetVolume(int nVolume)
{
	if (nVolume>255)
		nVolume=255;

	m_nVolume=m_nMaxVolume=nVolume;			

	nVolume=CalcSoundVolume(m_nVolume,1.0f);
	//if (m_pSSys->m_pCVarDebugSound->GetIVal()==5)
	//	m_pSSys->m_pILog->LogToConsole("VOL:%s,ch=%d,Set vol=%d",GetName(),m_nChannel,nVolume);
	ChangeVolume(nVolume);
}

//////////////////////////////////////////////////////////////////////
void CSound::SetConeAngles(float fInnerAngle,float fOuterAngle) 
{
	
}

//////////////////////////////////////////////////////////////////////
void CSound::SetPitch(int nValue)
{
	int iReScaledValue;
	// Scale from 0 - 1000 to FMOD range
	iReScaledValue = (int)((float)nValue / 1000.0f * (float)m_pSound->GetBaseFreq());
	iReScaledValue = __max(iReScaledValue, 100);
	// channel = The channel number/handle to change the frequency for. CS_ALL can also be used (see remarks).
    // freq = The frequency to set. Valid ranges are from 100 to 705600.
	if ((m_nChannel>=0) && (m_pSound->GetType()==btSAMPLE))
	{
		GUARD_HEAP;
 		CS_SetFrequency(m_nChannel, iReScaledValue);
	}
}

//////////////////////////////////////////////////////////////////////
void CSound::SetLoopPoints(const int iLoopStart, const int iLoopEnd)
{
	CHECK_LOADED(SetLoopPoints);
	if (m_pSound->GetSample())
	{
		GUARD_HEAP;
		CS_Sample_SetLoopPoints(m_pSound->GetSample(), iLoopStart, iLoopEnd);
	}
}

//////////////////////////////////////////////////////////////////////
void CSound::FXEnable(int nFxNumber)
{
	if (!m_pSSys->m_pCVarEnableSoundFX->GetIVal())
		return;
	if ((m_pSound->GetType()==btSAMPLE) && (m_nChannel!=-1))
	{
		GUARD_HEAP;
		m_nFxChannel=CS_FX_Enable(m_nChannel, nFxNumber);
	}
}

//////////////////////////////////////////////////////////////////////
void CSound::FXSetParamEQ(float fCenter,float fBandwidth,float fGain)
{

	if (!m_pSSys->m_pCVarEnableSoundFX->GetIVal())
		return;

	if ((m_pSound->GetType()==btSAMPLE) && (m_nFxChannel>=0))
	{
		GUARD_HEAP;
		CS_FX_SetParamEQ(m_nFxChannel, fCenter, fBandwidth, fGain);	
	}
}

//////////////////////////////////////////////////////////////////////
//! returns the size of the stream in ms
int CSound::GetLengthMs()
{
	GUARD_HEAP;
	if (m_pSound->NotLoaded())		
	{
		m_nFlags|=FLAG_SOUND_LOAD_SYNCHRONOUSLY; // force to load the sound

    if (!Preload())
    {
      //the sound buffer cannot be loaded
      if (!m_pSound->WaitForLoad())
        return (0);
    }
	}
	//CHECK_LOADED(GetLengthMs, 0);
	switch (m_pSound->GetType())
	{
		case btSTREAM:	return CS_Stream_GetLengthMs(m_pSound->GetStream());
		case btSAMPLE:	return (int)((float)CS_Sample_GetLength(m_pSound->GetSample())/(float)m_pSound->GetBaseFreq()*1000.0f);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////
//! returns the size of the stream in bytes
int CSound::GetLength()
{  
	GUARD_HEAP;
	if (m_pSound->NotLoaded())		
	{
		if (!Preload())
			return (0); //the sound buffer cannot be loaded
	}
	CHECK_LOADED(GetLengt, 0);
	if (m_pSound->GetStream())
		return CS_Stream_GetLength(m_pSound->GetStream());
	return 0;
  
//  return (GetLengthMs()*1000.0f);
}

//////////////////////////////////////////////////////////////////////
//! retrieves the currently played sample-pos, in milliseconds or bytes
unsigned int CSound::GetCurrentSamplePos(bool bMilliSeconds)
{
	GUARD_HEAP;
	if (m_pSound->NotLoaded())
	{
		if (!Preload())
			return (0); //the sound buffer cannot be loaded
	}
	//CHECK_LOADED(GetCurrentSamplePos, 0);
	switch (m_pSound->GetType())
	{
		case btSAMPLE:
			if (bMilliSeconds)
			{
				int nFreq;
				CS_Sample_GetDefaults(m_pSound->GetSample(), &nFreq, NULL, NULL, NULL);
				return (unsigned int)((float)CS_GetCurrentPosition(m_nChannel)/(float)nFreq*1000.0f);
			}else
			{
				return ((unsigned int)CS_GetCurrentPosition(m_nChannel));
			}
		case btSTREAM:
			if (bMilliSeconds)
				return ((unsigned int)CS_Stream_GetTime(m_pSound->GetStream()));
			else
				return ((unsigned int)CS_Stream_GetPosition(m_pSound->GetStream()));
	}
	return (0);
}

//! set the currently played sample-pos in bytes or milliseconds
//////////////////////////////////////////////////////////////////////
void CSound::SetCurrentSamplePos(unsigned int nPos,bool bMilliSeconds)
{
	GUARD_HEAP;
	if (m_pSound->NotLoaded())		
	{
		if (!Preload())
			return; //the sound buffer cannot be loaded
	}
	//CHECK_LOADED(SetCurrentSamplePos);
	switch (m_pSound->GetType())
	{
		case btSAMPLE:
			if (bMilliSeconds)
			{
				unsigned int nSample=(unsigned int)(((float)nPos/1000.0f)*(float)m_pSound->GetBaseFreq());
				if (IsPlayingOnChannel())
					CS_SetCurrentPosition(m_nChannel, nSample);
				else
					m_nStartOffset=nSample;
			}else
			{
				if (IsPlayingOnChannel())
					CS_SetCurrentPosition(m_nChannel,nPos);
				else
					m_nStartOffset=nPos;
			}
			break;
		case btSTREAM:
			if (bMilliSeconds)
				CS_Stream_SetTime(m_pSound->GetStream(),nPos);
			else
				CS_Stream_SetPosition(m_pSound->GetStream(),nPos);
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CSound::SetSoundProperties(float fFadingValue)
{
	m_fFadingValue = fFadingValue;
}

//////////////////////////////////////////////////////////////////////////
bool CSound::FadeIn()
{
	if (m_fCurrentFade >= 1)
	{
		m_fCurrentFade = 1;
		return true;
	}

	// this type of fading is purely framerate dependent
	m_fCurrentFade += m_fFadingValue*m_pSSys->m_pISystem->GetITimer()->GetFrameTime(); 
	
	//m_pSSys->m_pILog->LogToConsole("Fade IN, current fade=%f",m_fCurrentFade);
	if (m_fCurrentFade<0)
		m_fCurrentFade=0; 
	else
	{
		if (m_fCurrentFade>1.0f)
		{
			m_fCurrentFade=1.0f;		
			//m_nFadeType=0;
			return (true);
		}
	}
	return (false);
}

//////////////////////////////////////////////////////////////////////////
bool CSound::FadeOut()
{	
	// this type of fading is purely framerate dependent
	m_fCurrentFade-=m_fFadingValue*m_pSSys->m_pISystem->GetITimer()->GetFrameTime(); 

	//m_pSSys->m_pILog->LogToConsole("Fade OUT, current fade=%f",m_fCurrentFade);

	if (m_fCurrentFade<0)
	{
		m_fCurrentFade=0;		
		if (IsUsingChannel())
			FreeChannel();
		return (true);
	}
	else
		if (m_fCurrentFade>1.0f)
			m_fCurrentFade=1.0f;

	return (false);
}

//////////////////////////////////////////////////////////////////////////
void CSound::OnBufferLoaded()
{
	if (!m_bAlreadyLoaded)
	{
		OnEvent( SOUND_EVENT_ON_LOADED );

		if (m_bPlayAfterLoad)
		{		
			m_bPlayAfterLoad=false;
			Play(m_fPostLoadRatio,m_bPostLoadForceActiveState,m_bPostLoadSetRatio);
		}
	}
	m_bAlreadyLoaded = true;
}

//////////////////////////////////////////////////////////////////////////
void CSound::OnBufferLoadFailed()
{
	OnEvent( SOUND_EVENT_ON_LOAD_FAILED );
}

//////////////////////////////////////////////////////////////////////////
void CSound::GetMemoryUsage(class ICrySizer* pSizer)
{
	if (!pSizer->Add(*this))
		return;
	CSoundBuffer *pBuffer=m_pSound;
	pSizer->Add(*pBuffer);
}

//////////////////////////////////////////////////////////////////////////
bool CSound::IsLoading()
{
	if (m_pSound)
		return m_pSound->Loading();
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CSound::IsLoaded()
{
	if (m_pSound)
		return m_pSound->Loaded();
	return false;
}

bool CSound::IsUsingChannel()
{
	if (m_nChannel >=0)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CSound::AddEventListener( ISoundEventListener *pListener )
{
	m_listeners.insert(pListener );
}

//////////////////////////////////////////////////////////////////////////
void CSound::RemoveEventListener( ISoundEventListener *pListener )
{
	m_listeners.erase(pListener );
}

//////////////////////////////////////////////////////////////////////////
void CSound::OnEvent( ESoundCallbackEvent event )
{
	enum {THRESHOLD = 8};
	if (!m_listeners.empty())
	{
		size_t numListeners = m_listeners.size(); // set size is cached both in MS STL and STLPORT	
		if (numListeners == 1)
			(*m_listeners.begin())->OnSoundEvent (event, this); // single listener
		else
		if (numListeners < THRESHOLD)
		{
			// a small number of listeners
			ISoundEventListener* arrListeners[THRESHOLD];
			unsigned i = 0;
			for (Listeners::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it, ++i)
				arrListeners[i] = *it;

			// iterate through all callbacks
			for (i = 0; i < numListeners;++i)
			{
				ISoundEventListener* pListener = arrListeners[i];
				if (m_listeners.find(pListener) != m_listeners.end()) // this listener is still alive
					pListener->OnSoundEvent( event, this );
			}
		}
		else
		{
			// copy the current array of listeners and iterate through it
			// during each OnSoundEvent, the listener set may change, so validate
			// if the next listener is still in the set before calling upon it
			std::vector<ISoundEventListener*> arrListeners;
			arrListeners.reserve (numListeners);
			{
				// copy the array
				for (Listeners::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
					arrListeners.push_back(*it);
			}

			// iterate through all callbacks
			for (std::vector<ISoundEventListener*>::iterator it = arrListeners.begin(); it != arrListeners.end(); ++it)
			{
				ISoundEventListener* pListener = *it;
				if (m_listeners.find(pListener) != m_listeners.end()) // this listener is still alive
					pListener->OnSoundEvent( event, this );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSound::IsPlayLengthExpired( float fCurrTime  )
{
	if (m_fChannelPlayTime < 0 || m_bLoop || m_pSSys->m_bPause)
		return false;

	// Take few additional seconds precaution, (10 secs).
	if (fCurrTime > m_fChannelPlayTime+m_fSoundLengthSec + 10)
	{
		return true;
	}
	return false;
}

#pragma warning(default:4003)

#endif //_XBOX