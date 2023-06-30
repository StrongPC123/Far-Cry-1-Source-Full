/////////////////////////////////////////////////////////////////////////////////////////////////////
//	Crytek Character Animation source code
//	
//	History:
//  Taken over by Sergiy Migdalskiy (no previous history record here)
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "CryAnimation.h"
#include "CryBone.h"
#include "CryModelState.h"
#include "CryModEffector.h"
#include "CryModEffAnimation.h"
#include "CryModel.h"
#include "CVars.h"

CCryModEffAnimation::CCryModEffAnimation (CryModelState* pParent):
//	m_bMatrixPlusInUse(false),
	m_fAnimTime (0.0f),
	m_nAnimId (-1),
	m_fBlendInTime (0.0f),
	m_fBlendOutTime (0.0f),
	m_fOrigBlendOutTime (0.0f),
	m_pParent (pParent),
	m_fStopTime (0)
{
	m_uFlags.bLoop = 0;
	m_arrFadeAnims.reserve(2);
}

bool CCryModEffAnimation::IsStopped()
{
  return (m_nAnimId < 0) && m_arrFadeAnims.empty();
}

// does this animation effector actively plays some animations?
bool CCryModEffAnimation::isActive ()
{
	return m_nAnimId >= 0;
}


// this calls all the necessary callbacks
void CCryModEffAnimation::OnTimeChanged (int nAnimId, float fPrevTime, float fAnimTime,float fSpeed)
{	
	if (g_GetCVars()->ca_DisableAnimEvents())
		return;

  ICharInstanceSink * pSink = m_pParent->getAnimationEventSink(nAnimId);
	const AnimData& rAnim = m_pParent->getAnimationSet()->getAnimation(nAnimId);
	
	typedef CryModelState::AnimEventArray AnimEventArray;
	const AnimEventArray& arrEvents = m_pParent->getAnimEvents(nAnimId);
	// TODO: make binary search or iterator
	if(pSink && !arrEvents.empty() && fPrevTime != fAnimTime)
	{
		bool bSecondPass = false;
		float fSecondPassEnd;

		if (m_uFlags.bLoop
			&& rAnim.fStart < rAnim.fStop
			&& (fSpeed >= 0 ? fAnimTime > rAnim.fStop : fAnimTime < rAnim.fStart)
			)
		{
			bSecondPass = true;
			if (fSpeed >= 0)
			{
				fSecondPassEnd = rAnim.fStart + cry_fmod(fAnimTime - rAnim.fStart, rAnim.getLength());
				fAnimTime = rAnim.fStop;
			}
			else
			{
				fSecondPassEnd = rAnim.fStop - cry_fmod(rAnim.fStop - fAnimTime, rAnim.getLength());
				fAnimTime = rAnim.fStart;
			}
		}

		AnimEventArray::const_iterator it;
		for(it = arrEvents.begin(); it != arrEvents.end(); ++it)
		{
			float fTriggerTime = it->fTime;
			// this condition takes into account both variants, when the previous and current event are forward- or backward-playing
			if ( (fPrevTime <= fTriggerTime && fTriggerTime < fAnimTime)
				|| (fPrevTime >= fTriggerTime && fTriggerTime > fAnimTime))
			{
				DEFINE_PROFILER_SECTION("AllCallbacks");
				pSink->OnAnimationEvent (rAnim.strName.c_str(), it->UserData);
				if(g_GetCVars()->ca_Debug())
					g_GetLog()->Log("\004 %p->OnAnimationEvent (\"%s\",frame:%d,time:%g, trigger time: %g, time change: %g to %g)", m_pParent, rAnim.strName.c_str(), g_GetIRenderer()->GetFrameID(), fAnimTime, fTriggerTime, fPrevTime, fAnimTime);
			}
		}

		if (bSecondPass)
		{
			fAnimTime = fSecondPassEnd;
			fPrevTime = fSpeed >= 0 ? rAnim.fStart : rAnim.fStop;

			for(it = arrEvents.begin(); it != arrEvents.end(); ++it)
			{
				float fTriggerTime = it->fTime;
				if ( (fPrevTime <= fTriggerTime && fTriggerTime < fAnimTime)
					|| (fPrevTime >= fTriggerTime && fTriggerTime > fAnimTime))
				{
					DEFINE_PROFILER_SECTION("AllCallbacks");
					pSink->OnAnimationEvent (rAnim.strName.c_str(), it->UserData);
					if(g_GetCVars()->ca_Debug())
						g_GetLog()->Log("\004 %p->OnAnimationEvent(2nd) (\"%s\",frame:%d,time:%g, trigger time: %g, time change: %g to %g)", m_pParent, rAnim.strName.c_str(), g_GetIRenderer()->GetFrameID(), fAnimTime, fTriggerTime, fPrevTime, fAnimTime);
				}
			}
		}
	}
}


// returns fVal/fBase, clamped between 0 and 1
float blendValue (float fVal, float fBase)
{
	if (!(fVal > 0)) // this will also account for (impossible here) nans..
		return 0;
	else
	if (!(fVal < fBase))
		return 1;
	else
		return fVal/fBase;
}


void CCryModEffAnimation::SetNoLoop()
{
	m_uFlags.bLoop = 0;
}

void CCryModEffAnimation::SetNoLoopNoBlendOut()
{
	m_fBlendOutTime = 0;
	m_uFlags.bLoop = 0;
}


//////////////////////////////////////////////////////////////////////////
// Adds the current time of the animation
// Loops the animation if necessary
// Triggers all the necessary events (end animation and alike)
// Processes blending
// Returns the current animation time as it should be passed to the controller
unsigned CCryModEffAnimation::Tick(
	float fDeltaTime,
	const std::vector<ICharInstanceSink *>& arrSinks,
	CAnimationLayerInfoArray& arrOutLayers
)
{
	//Validator validator(this);
  if(m_nAnimId < 0)
		return TickFadingAnims(fDeltaTime,1,arrOutLayers);

	CryModelAnimationContainer* pAnimations = m_pParent->getAnimationSet();

	pAnimations->OnAnimationTick (m_nAnimId);
	const AnimData& rCurrentAnimation = pAnimations->getAnimation(m_nAnimId);

	// First animation

  float fStart = m_fStartTime;
  float fStop = m_fStopTime;

  ICharInstanceSink * pCharInstanceSink = m_pParent->getAnimationEventSink(m_nAnimId);

	if (!m_uFlags.bLoop && m_fAnimTime >= fStop + m_fBlendOutTime && fDeltaTime > 0)
	{
		// we have already applied the last frame of the last animation, so stop
		stop();
		return TickFadingAnims(fDeltaTime,1,arrOutLayers); // no animation
	}

	float fPrevTime = m_fAnimTime;
  m_fAnimTime += fDeltaTime;

  OnTimeChanged (m_nAnimId, fPrevTime, m_fAnimTime, fDeltaTime);
	
	float fCurrentTime;

  if(m_fAnimTime > fStop || m_fAnimTime < fStart)
  {
    if(m_uFlags.bLoop)
		{
			if (fStop > fStart)
			{
				if (fDeltaTime >= 0)
					m_fAnimTime = fStart + cry_fmod(m_fAnimTime - fStart, fStop-fStart);
				else
					m_fAnimTime = fStop - cry_fmod(fStop - m_fAnimTime, fStop-fStart);
			}
			else
			{
				if (fDeltaTime >= 0)
					m_fAnimTime = rCurrentAnimation.fStop;
				else
					m_fAnimTime = rCurrentAnimation.fStart;
			}
		}

		if (g_GetCVars()->ca_Debug())
			g_GetLog()->Log ("\003 %p->OnEndAnimation (%s)", m_pParent, rCurrentAnimation.strName.c_str());
    // The animation time intersected the stop time.
		// NOTE: this callback is not called a few times, if there were a few intersection (e.g. per frame)
		// because of a few cycles of animation passed.
    if(pCharInstanceSink)
		{
			DEFINE_PROFILER_SECTION("AllCallbacks");
			pCharInstanceSink->OnEndAnimation (rCurrentAnimation.strName.c_str());
		}

		if (!m_uFlags.bLoop)
		{
			if (g_GetCVars()->ca_TickVersion())
			{
				m_fAnimTime = fPrevTime; 
				stop();
				return TickFadingAnims(fDeltaTime,1,arrOutLayers);
			}
			else
			{
				fCurrentTime = fDeltaTime >= 0 ? fStop : fStart;
			}
		}
		else
		{
			fCurrentTime = m_fAnimTime;

			// in the looped animation, we don't have to have blend out time.
			// When the animation is looped, it ends only when another animation starts,
			// which will take care of blending this animation out (the new animation
			// will blend itself in)
			m_fBlendOutTime = 0;
		}
  }
	else
    fCurrentTime = m_fAnimTime;

	// what weight to use to blend the bone to the orientation/position dictated by the controller?
	// by default, it's 1, meaning fully replace the O/P of the bone with the one from the controller
	float fBlendWeight = 1;

	// Compute the current blend factor
	assert (m_fBlendInTime >= m_fBlendInCountdown);
	if (m_fBlendInCountdown > 0)
	{
		m_fBlendInCountdown -= (float)fabs(fDeltaTime);
		if (m_fBlendInCountdown < 0)
			m_fBlendInCountdown = 0;

		fBlendWeight = blendValue(m_fBlendInTime - m_fBlendInCountdown, m_fBlendInTime);
		if (!m_uFlags.bLoop)
		{
			if (fDeltaTime >= 0)
			{
				if (!(m_fAnimTime < fStop))
					m_fAnimTime = fStop;
			}
			else
			{
				if (!(m_fAnimTime > fStart))
					m_fAnimTime = fStart;
			}
		}
	}
	else
	if (m_fBlendOutTime > 0)
	{
		if (fDeltaTime >= 0)
		{
			if (m_fAnimTime > fStop)
				fBlendWeight = blendValue((fStop + m_fBlendOutTime) - m_fAnimTime, m_fBlendOutTime);
		}
		else
		{
			if (m_fAnimTime < fStart)
				fBlendWeight = blendValue(m_fAnimTime - (fStart - m_fBlendOutTime), m_fBlendOutTime);
		}
	}

	unsigned numFadingAnimations = 0;

	if (g_GetCVars()->ca_TickVersion() >= 2)
	{
		if (fBlendWeight >= 0.99f)
			m_arrFadeAnims.clear();
		else
			numFadingAnimations = TickFadingAnims(fDeltaTime, 1-fBlendWeight, arrOutLayers, m_fBlendInCountdown == 0);
	}
	else
		numFadingAnimations = TickFadingAnims(fDeltaTime, 1-fBlendWeight, arrOutLayers);

	//m_fAnimTime = fCurrentTime;
	arrOutLayers.push_back(CAnimationLayerInfo(m_nAnimId, fCurrentTime*getTicksPerSecond(), fBlendWeight));
	
	return 1 + numFadingAnimations;
}


float CCryModEffAnimation::getBlending()const
{
	if (m_fBlendInCountdown > 0)
		return (m_fBlendInTime - m_fBlendInCountdown) / m_fBlendInTime;
	else
	if (m_fBlendOutTime > 0 && m_fAnimTime > m_fStopTime)
		return (m_fStopTime + m_fBlendOutTime - m_fAnimTime) / m_fBlendOutTime;
	else
		return 1;
}

//////////////////////////////////////////////////////////////////////////
// Clocks all fading subanimations and adds the animation layers
// deletes expired fading animations
unsigned CCryModEffAnimation::TickFadingAnims (float fDelta, float fWeightLeft, CAnimationLayerInfoArray& arrLayers, bool bFade)
{
	g_arrLocalLayers.clear();

	FadingAnimArray::iterator it;
	for (it = m_arrFadeAnims.begin(); it != m_arrFadeAnims.end() && fWeightLeft > 0;)
	{
		if (it->Tick (fDelta, bFade))
		{
			if (it->fBlending >= fWeightLeft)
				g_arrLocalLayers.push_back(CAnimationLayerInfo(it->nAnimId, it->fTime*getTicksPerSecond(), 1));
			else
				g_arrLocalLayers.push_back(CAnimationLayerInfo(it->nAnimId, it->fTime*getTicksPerSecond(), blendValue(it->fBlending , fWeightLeft)));
			fWeightLeft -= it->fBlending;
			++it;
		}
		else
			it = m_arrFadeAnims.erase(it);
	}

	if (!m_arrFadeAnims.empty() && it != m_arrFadeAnims.end())
		m_arrFadeAnims.erase(it, m_arrFadeAnims.end());

	for (int i = (int)g_arrLocalLayers.size() - 1; i >= 0; --i)
		arrLayers.push_back(g_arrLocalLayers[i]);

	return (int)m_arrFadeAnims.size();
}


//////////////////////////////////////////////////////////////////////////
// returns the number of any animation currently being played
// this can be the current animation, including animation of fadeout, if the 
// layer doesn't have the current animation played, but has an animation fading out
int CCryModEffAnimation::GetAnyCurrentAnimation()const
{
	if (m_nAnimId < 0)
	{
		for (unsigned i = 0; i < m_arrFadeAnims.size(); ++i)
			if (m_arrFadeAnims[i].nAnimId >= 0)
				return m_arrFadeAnims[i].nAnimId;
		return -1; // no animations found at all
	}
	else
		return m_nAnimId;
}


float CCryModEffAnimation::GetPhase()const
{
	if (m_nAnimId >= 0)
	{
		CryModelAnimationContainer* pAnimations = m_pParent->getAnimationSet();
		const AnimData& anim = pAnimations->getAnimation(m_nAnimId);
		if (anim.bLoop && anim.fStart < anim.fStop)
			// only if the old animation is looped, and the new animation is looped, we use the phase
			return (m_fAnimTime - anim.fStart) / anim.getLength();
		else
			return 0; // there's no phase in non-looped (or 0-length) animation by the current definition
	}
	else
		return 0;// by default, there's no phase for non-existing non-playing animation
}

//////////////////////////////////////////////////////////////////////////
// Starts new animation
// Blends with the existing pose of the bone if necessary
void CCryModEffAnimation::StartAnimation (unsigned nAnimID, float fBlendInTime, float fBlendOutTime, CCryModEffAnimation* pSynchronizeWith, float fSpeed, unsigned nStartAnimFlags)
{
	if (nAnimID < 0)
	{
		Reset();
		return;
	}

	m_nStartAnimFlags = nStartAnimFlags;
	CryModelAnimationContainer* pAnimations = m_pParent->getAnimationSet();
	pAnimations->OnAnimationStart (nAnimID);
	// the animation might have changed
	const AnimData &anim = pAnimations->getAnimation(nAnimID);

	// ignore multiple requests to start the same animation unless the animation has already been played long enough (50% of the time)
	if (nAnimID == m_nAnimId)
	{
		if (g_GetCVars()->ca_RestartBehaviour() == 0)
		{
			if (fSpeed >= 0 ? m_fAnimTime < (anim.fStart+anim.fStop)/2 : m_fAnimTime > (anim.fStart+anim.fStop)/2)
				return;
		}
		else // with behaviour 1 always restart
			return;
	}

	// the phase of the old animation, 0..1, used to achieve smoother blending if the animations are
	// similar loops (only if they're both loops)
	float fOldAnimationPhase = pSynchronizeWith->GetPhase();

	// call OnEnd for animation we drop
	if (m_nAnimId >= 0)
	{
		if (g_GetCVars()->ca_Debug())
		{
			const AnimData& animOld = pAnimations->getAnimation(m_nAnimId);
			g_GetLog()->LogToFile ("\005Overriding existing animation #%u \"%s\" [%f..%f] at time %f", m_nAnimId, animOld.strName.c_str(), animOld.fStart, animOld.fStop, m_fAnimTime);
		}

		ICharInstanceSink * pCharInstanceSink = m_pParent->getAnimationEventSink(m_nAnimId);
		if (pCharInstanceSink)
		{
			DEFINE_PROFILER_SECTION("AllCallbacks");
			const char* szAnimName = pAnimations->getAnimation(m_nAnimId).strName.c_str();
			if (g_GetCVars()->ca_Debug())
				g_GetLog()->Log ("\003 %p->OnEndAnimation (%s)", m_pParent, szAnimName);
	    pCharInstanceSink->OnEndAnimation (szAnimName);
		}
	}

	this->stop();

	m_fStartTime = anim.fStart;
	m_fStopTime = anim.fStop;
	m_uFlags.bLoop = anim.bLoop ? 1 : 0;

	// set new animation
	m_nAnimId   = nAnimID;
	
	m_fAnimTime = anim.bLoop ? anim.fStart + fOldAnimationPhase * anim.getLength() : fSpeed > 0 ? m_fStartTime : m_fStopTime;

	// start blending from old to new if blending allowed
	m_fBlendInCountdown = m_fBlendInTime = fBlendInTime;
	m_fOrigBlendOutTime = m_fBlendOutTime = fBlendOutTime;

	ICharInstanceSink* pSink = m_pParent->getAnimationEventSink(nAnimID);

	if (pSink)
	{
		DEFINE_PROFILER_SECTION("AllCallbacks");
		const char *szAnimName = anim.strName.c_str();
		if (g_GetCVars()->ca_Debug())
			g_GetLog()->Log ("\003 %p->OnStartAnimation (%s)", m_pParent, szAnimName);
		pSink->OnStartAnimation (szAnimName);
	}
}

void CCryModEffAnimation::stop ()
{
	if (m_nAnimId < 0)
		return;

	if (m_fOrigBlendOutTime > 0)
	{
		FadingAnim anim;
		anim.fBlending      = getBlending();
		anim.fBlendOutSpeed = 1/m_fOrigBlendOutTime;
		anim.fTime          = m_fAnimTime;
		anim.nAnimId        = m_nAnimId;
		anim.bLoop = m_uFlags.bLoop?true:false;

		anim.bRun = anim.bLoop || (m_nStartAnimFlags&CryCharAnimationParams::FLAGS_ALIGNED);

		CryModelAnimationContainer* pAnimations = m_pParent->getAnimationSet();
		const AnimData& rCurrentAnimation = pAnimations->getAnimation(m_nAnimId);
		anim.fLoopEnd       = rCurrentAnimation.fStop;
		anim.fLoopLength     = rCurrentAnimation.getLength();
		
		m_arrFadeAnims.insert(m_arrFadeAnims.begin(), anim);
	}

	// small safety check in case the queue gets really long...
	if (g_GetCVars()->ca_TickVersion() >= 2 && m_arrFadeAnims.size() > 8)
		m_arrFadeAnims.resize (8);

	//m_uFlags.bLoop = 0;
	//m_fStopTime = m_fAnimTime;
	//m_fBlendOutTime = m_fOrigBlendOutTime;
	m_nAnimId = -1;
}

// forcibly sets the current time of the animation, in seconds
void CCryModEffAnimation::SetCurrentTime (float fTime)
{
	//m_fBlendInTime = m_fBlendOutTime = 0;
	if (m_nAnimId < 0)
		return;

	// filter out the out-of-range values
	if (fTime < m_fStartTime)
		fTime = m_fStartTime;
	else
	if (fTime > m_fStopTime)
		fTime = m_fStopTime;

	m_arrFadeAnims.clear();

	float fPrevTime = m_fAnimTime;
	m_fAnimTime = fTime;

	// so as to avoid calling extra callbacks
	OnTimeChanged (m_nAnimId, fPrevTime, m_fAnimTime, fTime-fPrevTime);
}

void CCryModEffAnimation::Reset()
{
  m_fBlendInCountdown = m_fBlendInTime = m_fBlendOutTime = 0.f;
  m_nAnimId   = -1;
	m_arrFadeAnims.clear();
}

float CCryModEffAnimation::getTicksPerSecond()const
{
	CryModelAnimationContainer* pAnimations = m_pParent->getAnimationSet();
	return pAnimations->getTicksPerSecond();
}

// ticks the fading animation; returns false when the animation completely fades out
bool CCryModEffAnimation::FadingAnim::Tick (float fDeltaTime, bool bFade)
{
	if (bRun)
	{
		if (bLoop)
		{
			if (fLoopLength)
			{
				float fLoopStart = fLoopEnd - fLoopLength;
				if (fDeltaTime > 0)
					fTime = fLoopStart + cry_fmod ((fTime + fDeltaTime) - fLoopStart, fLoopLength);
				else
					fTime = fLoopEnd - cry_fmod (fLoopEnd - (fTime + fDeltaTime), fLoopLength);
			}
		}
		else
		{
			fTime += fDeltaTime;
		}
	}

	if (bFade)
		fBlending -= cry_fabsf(fDeltaTime*fBlendOutSpeed);
	return fBlending > 0;
}

void CCryModEffAnimation::initClass()
{
	g_arrLocalLayers.reserve (3);
}

string CCryModEffAnimation::dump()
{
	CryModelAnimationContainer* pAnimations = m_pParent->getAnimationSet();
	const AnimData& rAnim = pAnimations->getAnimation(m_nAnimId);
	string strResult;
	char szBuf[0x100];
	strResult += "main \"" + rAnim.strName + "\"";
	sprintf (szBuf, " t=%.2f", m_fAnimTime);
	strResult += szBuf;
	if (!m_arrFadeAnims.empty())
	{
		strResult += ", fade";
		for (FadingAnimArray::iterator it = m_arrFadeAnims.begin(); it != m_arrFadeAnims.end(); ++it)
		{
			const AnimData& fadeAnim = pAnimations->getAnimation(it->nAnimId);
			sprintf (szBuf, " \"%s\" (t=%.2f b=%.2f)", fadeAnim.strName.c_str(), it->fTime, it->fBlending);
			strResult += szBuf;
		}
	}
	return strResult;
}

void CCryModEffAnimation::deinitClass()
{
	g_arrLocalLayers.clear();
}
CAnimationLayerInfoArray CCryModEffAnimation::g_arrLocalLayers;
